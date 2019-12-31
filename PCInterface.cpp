/*
 * PCInterface.cpp
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#include "PCInterface.h"

extern DSerial serial;
//bool interruptEnabled = false;

PCInterface *instancePCInterface;
enum InternalState { firstByte, secondByte };
InternalState status;
unsigned short tmpValue;

void PCInterface_IRQHandler( void )
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus( instancePCInterface->module );
    //MAP_UART_clearInterruptFlag( instancePCInterface->module, status );

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        // new byte received
        instancePCInterface->rxQueue.push( MAP_UART_receiveData( instancePCInterface->module ));
    }
}

void taskCallback( void )
{
    while ( !instancePCInterface->rxQueue.empty() )
    {
        // data has been received
        unsigned char data;
        instancePCInterface->rxQueue.pop(data);

        // were we waiting for the first byte?
        // did we receive the first byte?
        if ((status == firstByte) && (data & FIRST_BYTE))
        {
            tmpValue = ((unsigned short)data) << 8;
            status = secondByte;
        }
        else if ((status == secondByte) && !(data & FIRST_BYTE))
        {
            tmpValue |= (unsigned short)data;
            if (instancePCInterface->user_onReceive)
            {
                /*serial.print("processed ");
                serial.print(tmpValue, HEX);
                serial.println();*/
                instancePCInterface->user_onReceive(tmpValue);
            }
            status = firstByte;
        }
        // otherwise ignore the byte received
    }
}

PCInterface::PCInterface() : Task(&taskCallback)
{
    module = EUSCI_A1_BASE;
    modulePort = GPIO_PORT_P2;
    modulePins = GPIO_PIN2 | GPIO_PIN3;

    user_onReceive = 0;

    status = firstByte;
    tmpValue = 0;

    // store the pointer in a static variable
    instancePCInterface = this;
}

void PCInterface::init(unsigned int baudrate)
{
    MAP_UART_disableModule(module);   //disable UART operation for configuration settings

    // transmit / receive interrupt request handler
    MAP_UART_registerInterrupt(module, PCInterface_IRQHandler);

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, modulePins, GPIO_PRIMARY_MODULE_FUNCTION);

    eUSCI_UART_Config Config;

    //Default Configuration, macro found in uart.h
    Config.selectClockSource    = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    Config.parity               = EUSCI_A_UART_NO_PARITY;
    Config.msborLsbFirst        = EUSCI_A_UART_LSB_FIRST;
    Config.numberofStopBits     = EUSCI_A_UART_ONE_STOP_BIT;
    Config.uartMode             = EUSCI_A_UART_MODE;

    unsigned int n = MAP_CS_getSMCLK() / baudrate;

    if (n > 16)
    {
        Config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;    // Oversampling
        Config.clockPrescalar = n >> 4;                                         // BRDIV = n / 16
        Config.firstModReg = n - (Config.clockPrescalar << 4);                  // UCxBRF = int((n / 16) - int(n / 16)) * 16
    }
    else
    {
        Config.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;   // Oversampling
        Config.clockPrescalar = n;                                              // BRDIV = n
        Config.firstModReg = 0;                                                 // UCxBRF not used
    }

    Config.secondModReg = 0;                                                    // UCxBRS = 0

    MAP_UART_initModule(module, &Config);

    MAP_UART_enableModule(module);                                              // enable UART operation
}

void PCInterface::setReceptionHandler( void (*hnd)( unsigned short data ))
{
    user_onReceive = hnd;
    if (hnd)
    {
        uint32_t status = MAP_UART_getEnabledInterruptStatus(this->module);

        // clear the receive interrupt to avoid spurious triggers the first time
        MAP_UART_clearInterruptFlag( this->module, status );

        // enable the interrupt
        MAP_UART_enableInterrupt( module, EUSCI_A_UART_RECEIVE_INTERRUPT );         // enable RX interrupt
    }
    else
    {
        // disable the interrupt
        MAP_UART_disableInterrupt( module, EUSCI_A_UART_RECEIVE_INTERRUPT );         // enable RX interrupt
    }
}

void PCInterface::send( unsigned short data )
{
    MAP_UART_transmitData( module, (data >> 8) & 0x7F | 0x80 );
    MAP_UART_transmitData( module, data & 0xFF );
}

void PCInterface::executeTask()
{
    if (!instancePCInterface->rxQueue.empty())
    {
        if (userFunction)
        {
            userFunction();
        }
        execute = false;
    }
}
