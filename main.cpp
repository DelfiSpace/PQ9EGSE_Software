#include "EGSE.h"


// tasks
PeriodicTask timerTask(200, periodicTask); //flash LED 5 times per second
PeriodicTask* periodicTasks[] = {&timerTask};
PeriodicTaskNotifier taskNotifier = PeriodicTaskNotifier(periodicTasks, 1);

PCInterface pcInterface;
HWInterface hwInterface;
Task* tasks[] = { &timerTask, &pcInterface, &hwInterface };


// TODO: remove when bug in CCS has been solved
void PCreceivedByte(unsigned short data)
{
    hwInterface.send(data);
}

// TODO: remove when bug in CCS has been solved
void PQ9receivedByte( unsigned short data )
{
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
    // stop internal watch-dog
    MAP_WDT_A_holdTimer();

    // initialize the MCU:
    // - clock source
    // - clock tree
    DelfiPQcore::initMCU();

    Console::init( 115200 );                        // baud rate: 115200 bps

    // initialize Task Notifier
    taskNotifier.init();

    // initialize the interfaces
    pcInterface.init(230400);
    hwInterface.init(HWInterface::PQ9);

    // link the command handlers to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    pcInterface.setReceptionHandler(PCreceivedByte);
    hwInterface.setReceptionHandler(PQ9receivedByte);

    // initialize activity monitor LED
    MAP_GPIO_setOutputHighOnPin( GPIO_PORT_P10, GPIO_PIN5 );
    MAP_GPIO_setAsOutputPin( GPIO_PORT_P10, GPIO_PIN5 );

    //Initialize reset pin
    MAP_GPIO_setOutputLowOnPin( GPIO_PORT_P2, GPIO_PIN0 );
    MAP_GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN0 );

    Console::log("EGSE booting....");

    TaskManager::start(tasks, 3);
}
