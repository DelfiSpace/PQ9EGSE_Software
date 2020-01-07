/*
 * PCInterface.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef PCINTERFACE_H_
#define PCINTERFACE_H_

#include <driverlib.h>
#include "DSerial.h"
#include "Queue.h"
#include "Task.h"

#define FIRST_BYTE                   0x80
#define ADDRESS_BIT                  0x40
#define STOP_TRANSMISSION            0x20

class PCInterface: public Task
{
private:
    uint32_t module;
    uint8_t modulePort;
    uint16_t modulePins;
    Queue<unsigned char> rxQueue;
    void (*user_onReceive)( unsigned short data );

    friend void PCInterface_IRQHandler();
    friend void taskCallback();

protected:
    //virtual bool notified();
    virtual void run();
    virtual void setUp();

public:
    PCInterface();
    void init(unsigned int baudrate);
    void setReceptionHandler( void (*hnd)( unsigned short data ));
    void send( unsigned short data);
};

#endif /* PCINTERFACE_H_ */
