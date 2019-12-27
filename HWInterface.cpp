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

        if (MAP_UART_queryStatusFlags( instancePQ9Interface->module, EUSCI_A_UART_ADDRESS_RECEIVED ))
        {
            unsigned char data = MAP_UART_receiveData( instancePQ9Interface->module );
            // This is an address bit
            instancePQ9Interface->rxQueue.push( 0x4000 | ((data & 0x80) << 1) | (data & 0x7F));
        }
        else
        {
            unsigned char data = MAP_UART_receiveData( instancePQ9Interface->module );
            // new byte received
            instancePQ9Interface->rxQueue.push(((data & 0x80) << 1) | (data & 0x7F));
        }
    }
}

void PQ9taskCallback( void )
{
    while ( !instancePQ9Interface->rxQueue.empty() )
    {


        // data has been received
        unsigned short data;
        instancePQ9Interface->rxQueue.pop(data);

        serial.print("PQ9 RX ");
        serial.print(data, HEX);
        serial.println();

        instancePQ9Interface->user_onReceive(data);


    }
}

HWInterface::HWInterface() : Task(&PQ9taskCallback)
{
    module = EUSCI_A3_BASE;
    modulePort = GPIO_PORT_P9;
    modulePins = GPIO_PIN6 | GPIO_PIN7;

    TXEnablePort = GPIO_PORT_P9;
    TXEnablePin = GPIO_PIN0;

    // store the pointer in a static variable
    instancePQ9Interface = this;
}

void HWInterface::init( unsigned int baudrate )
{
    MAP_UART_disableModule( module );   //disable UART operation for configuration settings

    this->baudrate = baudrate;

    // transmit / receive interrupt request handler
    MAP_UART_registerInterrupt( module, PQ9Interface_IRQHandler );

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, modulePins, GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin( TXEnablePort,
                                    TXEnablePin, GPIO_PRIMARY_MODULE_FUNCTION );

    MAP_GPIO_setOutputLowOnPin( TXEnablePort, TXEnablePin );

    MAP_GPIO_setAsOutputPin( TXEnablePort, TXEnablePin );

    eUSCI_UART_Config Config;

    //Default Configuration, macro found in uart.h
    Config.selectClockSource    = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    Config.parity               = EUSCI_A_UART_NO_PARITY;
    Config.msborLsbFirst        = EUSCI_A_UART_LSB_FIRST;
    Config.numberofStopBits     = EUSCI_A_UART_ONE_STOP_BIT;
    Config.uartMode             = EUSCI_A_UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE;

    unsigned int n = MAP_CS_getSMCLK() / baudrate;

    if (n > 16)
    {
        Config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;        // Oversampling
        Config.clockPrescalar = n >> 4;                                             // BRDIV = n / 16
        Config.firstModReg = n - (Config.clockPrescalar << 4);                      // UCxBRF = int((n / 16) - int(n / 16)) * 16
    }
    else
    {
        Config.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;       // Oversampling
        Config.clockPrescalar = n;                                                  // BRDIV = n
        Config.firstModReg = 0;                                                     // UCxBRF not used
    }

    Config.secondModReg = 0;                                                        // UCxBRS = 0

    MAP_UART_initModule( module, &Config) ;

    MAP_UART_enableModule( module );                                                // enable UART operation
}

void HWInterface::setReceptionHandler( void (*hnd)( unsigned short data ))
{
    user_onReceive = hnd; //parse handler function

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

void HWInterface::send( unsigned short data)
{
    txQueue.push( data );

    uint32_t status = MAP_UART_getEnabledInterruptStatus( module );
    //if (!interruptEnabled1)
    {


        unsigned short first;
        bool ret = txQueue.pop(first);

        if (ret)
        {
            unsigned char data = ((first >> 1) & 0x80) | (first & 0x7F);
            unsigned char cmd = (first & 0xFE00) >> 8;

            if (!(cmd & STOP_TRANSMISSION))
            {
                MAP_GPIO_setOutputHighOnPin( TXEnablePort, TXEnablePin );
            }
            /*serial.print("transmitNext ");
            serial.print(data, HEX);
            serial.print(" ");
            serial.print(cmd, HEX);
            serial.println();*/


            if ( cmd & ADDRESS_BIT )
            {
                // address
                MAP_UART_transmitAddress( module, data );
            }
            else
            {

                // simple value
                MAP_UART_transmitData( module, data );
            }


        if (cmd & STOP_TRANSMISSION)
                {

        // Workaround for USCI42 errata
        // introduce a 2 bytes delay to make sure the UART buffer is flushed
        uint32_t d = MAP_CS_getMCLK() * 4 / instancePQ9Interface->baudrate;
        for(uint32_t k = 0; k < d;  k++)
        {
            __asm("  nop");
        }

        MAP_GPIO_setOutputLowOnPin( instancePQ9Interface->TXEnablePort, instancePQ9Interface->TXEnablePin );
        //MAP_UART_disableInterrupt( instancePQ9Interface->module, EUSCI_A_UART_TRANSMIT_INTERRUPT );
                }
        }
        // enable the interrupt
        //MAP_UART_enableInterrupt( module, EUSCI_A_UART_TRANSMIT_INTERRUPT );
        //interruptEnabled1 = true;
    }
}

void HWInterface::executeTask()
{
    if (!instancePQ9Interface->rxQueue.empty())
    {
        if (userFunction)
        {
            userFunction();
        }
        execute = false;
    }
}
