#include "EGSE.h"

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);

// debug console handler
DSerial serial;

// tasks
PeriodicTask timerTask(FCLOCK, periodicTask);
Task* tasks[] = { &timerTask };

// TODO: remove when bug in CCS has been solved
void receivedCommand(PQ9Frame &newFrame)
{
    serial.println("PQ9Frame received");
    //cmdHandler.received(newFrame);
}

void periodicTask()
{
    // flash LED
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
    pq9bus.begin(115200, EGSE_ADDRESS);     // baud rate: 115200 bps
                                            // address EGSE (8)

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    //pq9bus.setReceiveHandler([](PQ9Frame &newFrame){ cmdHandler.received(newFrame); });
    pq9bus.setReceiveHandler(receivedCommand);

    serial.println("EGSE booting...");

    TaskManager::start(tasks, 2);
}
