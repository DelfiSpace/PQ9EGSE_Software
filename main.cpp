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
    hwInterface.send(data);
}

void PQ9receivedFrame( unsigned short data )
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

    // Change VCORE to 1 to support the 48MHz frequency
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    //Setting the DCO Frequency
    MAP_CS_setDCOFrequency(48000000);

    // Configure clocks that we need
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);

    serial.begin( );                        // baud rate: 9600 bps

    // initialize the interfaces
    pcInterface.init(115200 * 2);
    hwInterface.init(9600, HWInterface::RS485);

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
