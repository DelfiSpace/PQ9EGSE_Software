/*
 * HWInterface.cpp
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#include <HWInterface.h>

HWInterface *instancePQ9Interface;
extern DSerial serial;

void PQ9Interface_IRQHandler( void )
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus( instancePQ9Interface->module );

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        uint_fast8_t addressStatus = MAP_UART_queryStatusFlags( instancePQ9Interface->module, EUSCI_A_UART_ADDRESS_RECEIVED );
        unsigned short data = MAP_UART_receiveData( instancePQ9Interface->module );

        if ( addressStatus )
        {
            // This is an address bit
            instancePQ9Interface->rxQueue.push( ADDRESS_BIT | ((data & 0x80) << 1) | (data & 0x7F));
        }
        else
        {
            // new byte received
            instancePQ9Interface->rxQueue.push( ((data & 0x80) << 1) | (data & 0x7F));
        }
        instancePQ9Interface->notify();
    }
}

void HWInterface::run( void )
{
    while ( !instancePQ9Interface->rxQueue.empty() )
    {
        // data has been received
        unsigned short data;
        instancePQ9Interface->rxQueue.pop(data);
        instancePQ9Interface->user_onReceive(data);
    }
}

HWInterface::HWInterface() : Task()
{
    module = EUSCI_A3_BASE;
    modulePort = GPIO_PORT_P9;
    modulePins = GPIO_PIN6 | GPIO_PIN7;

    TXEnablePort = GPIO_PORT_P9;
    TXEnablePin = GPIO_PIN0;

    user_onReceive = 0;
    baudrate = 115200;

    // store the pointer in a static variable
    instancePQ9Interface = this;
}

void HWInterface::init( InterfaceType interface )
{
    if (interface == HWInterface::RS485)
    {
        serial.println("RS485 mode");
    }
    else
    {
        serial.println("PQ9 mode");
    }

    MAP_UART_disableModule( module );   //disable UART operation for configuration settings

    MAP_UART_clearInterruptFlag( module, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG );

    // transmit / receive interrupt request handler
    MAP_UART_registerInterrupt( module, &PQ9Interface_IRQHandler );

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, modulePins, GPIO_PRIMARY_MODULE_FUNCTION);
    //MAP_GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    //MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(modulePort, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_GPIO_setOutputLowOnPin( TXEnablePort, TXEnablePin );
    MAP_GPIO_setAsOutputPin( TXEnablePort, TXEnablePin );

    eUSCI_UART_Config Config;

    //Default Configuration, macro found in uart.h
    Config.selectClockSource    = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    Config.parity               = EUSCI_A_UART_NO_PARITY;
    Config.msborLsbFirst        = EUSCI_A_UART_LSB_FIRST;
    Config.numberofStopBits     = EUSCI_A_UART_ONE_STOP_BIT;

    if (interface == HWInterface::RS485)
    {
        Config.uartMode             = EUSCI_A_UART_MODE;
        baudrate                    = 9600;
    }
    else
    {
        Config.uartMode             = EUSCI_A_UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE;
        baudrate                    = 115200;
    }

    unsigned int n = MAP_CS_getSMCLK() / baudrate;

    if (n > 16)
    {
        Config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;        // Oversampling
        Config.clockPrescalar = n >> 4;                                             // BRDIV = n / 16
        Config.firstModReg = n - (Config.clockPrescalar << 4);                      // UCxBRF = int((n / 16) - int(n / 16)) * 16
    }
    else
    {
        Config.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;       // Low-frequency mode
        Config.clockPrescalar = n;                                                  // BRDIV = n
        Config.firstModReg = 0;                                                     // UCxBRF not used
    }

    Config.secondModReg = 0;                                                        // UCxBRS = 0

    MAP_UART_initModule( module, &Config) ;

    MAP_UART_enableModule( module );                                                // enable UART operation

    // if we are re-initializing, re-enable the receive interrupt
    if ( user_onReceive )
    {
        MAP_UART_enableInterrupt( module, EUSCI_A_UART_RECEIVE_INTERRUPT );
    }
}

void HWInterface::setReceptionHandler( void (*hnd)( unsigned short data ))
{
    user_onReceive = hnd;

    if ( hnd )
    {
        uint32_t status = MAP_UART_getEnabledInterruptStatus( module );

        // clear the receive interrupt to avoid spurious triggers the first time
        MAP_UART_clearInterruptFlag( module, status );

        // enable receive interrupt
        MAP_UART_enableInterrupt( module, EUSCI_A_UART_RECEIVE_INTERRUPT );
    }
    else
    {
        // disable receive interrupt
        MAP_UART_disableInterrupt( module, EUSCI_A_UART_RECEIVE_INTERRUPT );
    }
}

void HWInterface::send( unsigned short input )
{
    unsigned char data = ((input >> 1) & 0x80) | (input & 0x7F);

    // process the command
    if (input & COMMAND)
    {
        if (data == INTERFACE_PQ9)
        {
            init( HWInterface::PQ9 );
        }
        else if (data == INTERFACE_RS485)
        {
            init( HWInterface::RS485 );
        }
        return;
    }

    // turn the transmit enable on
    if (!(input & STOP_TRANSMISSION))
    {
        MAP_GPIO_setOutputHighOnPin( TXEnablePort, TXEnablePin );
    }

    if ( input & ADDRESS_BIT )
    {
        // address
        MAP_UART_transmitAddress( module, data );
    }
    else
    {
        // simple value
        MAP_UART_transmitData( module, data );
    }

    // if this is the last byte to transmit, turn off the transmit enable
    if (input & STOP_TRANSMISSION)
    {
        // Workaround for USCI42 errata
        // introduce a small delay to make sure the UART buffer is flushed
        uint32_t d = MAP_CS_getMCLK() * 3 / baudrate;
        for(uint32_t k = 0; k < d;  k++)
        {
            __asm("  nop");
        }

        MAP_GPIO_setOutputLowOnPin( TXEnablePort, TXEnablePin );
    }
}
