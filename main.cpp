#include "EGSE.h"

// debug console handler
DSerial serial;

// tasks
PeriodicTask timerTask(FCLOCK / 5, periodicTask);               // Flash LED 5 times per second
PCInterface pcInterface;
HWInterface hwInterface;
Task* tasks[] = { &timerTask, &pcInterface, &hwInterface };

// TODO: remove when bug in CCS has been solved
void PCreceivedFrame(unsigned short data)
{
    serial.print("PC ");
    serial.print(data, HEX);
    serial.println();
    hwInterface.send(data);
}

void PQ9receivedFrame( unsigned short data )
{
    serial.print("PQ9 ");
    serial.print(data, HEX);
    serial.println();
    pcInterface.send(data);
}

void periodicTask()
{
    // toggle LED
    MAP_GPIO_toggleOutputOnPin( GPIO_PORT_P10, GPIO_PIN5 );
}

/**
 * main.c
 */
void main(void)
{
    // initialize the MCU:
    // - clock source
    // - clock tree
    DelfiPQcore::initMCU();

    serial.begin( );                        // baud rate: 9600 bps

    // initialize the interfaces
    pcInterface.init(115200 * 2);
    hwInterface.init(1200, HWInterface::RS485);

    // link the command handlers to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    pcInterface.setReceptionHandler(PCreceivedFrame);
    hwInterface.setReceptionHandler(PQ9receivedFrame);

    // initialize activity monitor LED
    MAP_GPIO_setOutputHighOnPin( GPIO_PORT_P10, GPIO_PIN5 );
    MAP_GPIO_setAsOutputPin( GPIO_PORT_P10, GPIO_PIN5 );

    serial.println("EGSE booting...");

    TaskManager::start(tasks, 3);
}
