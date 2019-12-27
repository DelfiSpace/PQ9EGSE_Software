#include "EGSE.h"

// CDHS bus handler
//PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);

// debug console handler
DSerial serial;

// tasks
PeriodicTask timerTask(FCLOCK / 5, periodicTask);       // Flash LED twice per second
PCInterface pcInterface;
PQ9Interface pq9Interface;
Task* tasks[] = { &timerTask, &pcInterface, &pq9Interface };

// TODO: remove when bug in CCS has been solved
void PCreceivedFrame(unsigned char *data, unsigned short size)
{
    serial.println("PC");
    for(int i = 0; i < size; i++)
    {
        serial.print(data[i], HEX);
        serial.print(" ");
    }
    serial.println();
    pq9Interface.send(data, size);
}

void PQ9receivedFrame( unsigned char data )
{
    pcInterface.send(data);
    /*serial.println("PQ9");
    for(int i = 0; i < size; i++)
    {
        serial.print(data[i], HEX);
        serial.print(" ");
    }
    serial.println();
    pq9Interface.send(data, size);*/
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
//    pq9bus.begin(115200, EGSE_ADDRESS);     // baud rate: 115200 bps
                                            // address EGSE (8)

    // initialize the interfaces
    pcInterface.init(115200);
    pq9Interface.init(115200);

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    //pq9bus.setReceiveHandler([](PQ9Frame &newFrame){ cmdHandler.received(newFrame); });
    pcInterface.setReceptionHandler(PCreceivedFrame);
    pq9Interface.setReceptionHandler(PQ9receivedFrame);


    // initialize activity monitor LED
    MAP_GPIO_setOutputHighOnPin( GPIO_PORT_P10, GPIO_PIN5 );
    MAP_GPIO_setAsOutputPin( GPIO_PORT_P10, GPIO_PIN5 );

    serial.println("EGSE booting...");

    TaskManager::start(tasks, 3);
}
