/*
 * PQ9Interface.cpp
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#include "PQ9Interface.h"

PQ9Interface *instancePQ9Interface;
extern DSerial serial;
unsigned char counter = 0;

void PQ9Interface_IRQHandler( void )
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus( instancePQ9Interface->module );
    MAP_UART_clearInterruptFlag( instancePQ9Interface->module, status );

    if (status & UCRXIFG)
    {
        // new byte received
    instancePQ9Interface->rxQueue.push( MAP_UART_receiveData( instancePQ9Interface->module ));
    }

}

void PQ9taskCallback( void )
{
    while ( !instancePQ9Interface->rxQueue.empty() )
    {
        // data has been received
        unsigned char data;
        instancePQ9Interface->rxQueue.pop(data);

        instancePQ9Interface->user_onReceive(data);
        //serial.print("PQ9 ");
        //serial.print(data, HEX);
        //serial.println();
    }
}

PQ9Interface::PQ9Interface() : HWInterface(&PQ9taskCallback)
{
    module = EUSCI_A3_BASE;
    modulePort = GPIO_PORT_P9;
    modulePins = GPIO_PIN6 | GPIO_PIN7;

    TXEnablePort = GPIO_PORT_P9;
    TXEnablePin = GPIO_PIN0;

    // store the pointer in a static variable
    instancePQ9Interface = this;
}

void PQ9Interface::init( unsigned int baudrate )
{
    MAP_UART_disableModule( module );   //disable UART operation for configuration settings

    this->baudrate = baudrate;
    unsigned char address = 7;

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
    //MAP_UART_setDormant( module );                                                  // address bit multi processor mode,
                                                                                    // only address will triggered RXIFG
}

void PQ9Interface::setReceptionHandler( void (*hnd)( unsigned char data ))
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

void PQ9Interface::send( unsigned char *data, unsigned short size)
{
    serial.print("PQ9 RX ");
    serial.print(counter, DEC);
    serial.println();
    MAP_GPIO_setOutputHighOnPin( TXEnablePort, TXEnablePin );

    MAP_UART_transmitAddress(this->module, data[0]);

    for (int i = 0; i < data[1] + 4; i++)
    {
        MAP_UART_transmitData(this->module, data[i + 1]);
    }

    // Workaround for USCI42 errata
    // introduce a 2 bytes delay to make sure the UART buffer is flushed
    uint32_t d = MAP_CS_getMCLK() * 4 / baudrate;
    for(uint32_t k = 0; k < d;  k++)
    {
        __asm("  nop");
    }

    MAP_GPIO_setOutputLowOnPin( TXEnablePort, TXEnablePin );
}

void PQ9Interface::executeTask()
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
