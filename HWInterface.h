/*
 * HWInterface.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef HWINTERFACE_H_
#define HWINTERFACE_H_

#include <driverlib.h>
#include "Task.h"
#include "DSerial.h"
#include "Queue.h"


#define FIRST_BYTE                   0x80
#define ADDRESS_BIT                  0x40
#define STOP_TRANSMISSION            0x20

class HWInterface: public Task
{
private:
    uint32_t module;
    uint8_t modulePort;
    uint16_t modulePins;
    unsigned long TXEnablePort;
    unsigned long TXEnablePin;
    unsigned int baudrate;
    void (*user_onReceive)( unsigned short data );
    Queue<unsigned short> rxQueue;
    Queue<unsigned short> txQueue;

    friend void PQ9Interface_IRQHandler( void );
    friend void PQ9taskCallback();

protected:
    virtual void executeTask();

public:
    HWInterface();
    void init( unsigned int baudrate );
    void setReceptionHandler( void (*hnd)( unsigned short data ));
    void send( unsigned short data);
};

#endif /* HWINTERFACE_H_ */
