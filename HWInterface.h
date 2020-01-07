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
#define COMMAND                      0x10
#define INITIALIZE                   0x00
#define INTERFACE_PQ9                0x01
#define INTERFACE_RS485              0x02

class HWInterface: public Task
{
private:
    uint32_t module;
    uint8_t modulePort;
    uint16_t modulePins;
    unsigned long TXEnablePort;
    unsigned long TXEnablePin;
    unsigned int baudrate;
    void (*user_onReceive)( unsigned short input );
    Queue<unsigned short> rxQueue;

    friend void PQ9Interface_IRQHandler( void );
    friend void PQ9taskCallback();

protected:
    virtual bool notified();
    virtual void run();

public:
    enum InterfaceType {PQ9, RS485};
    HWInterface();
    void init( InterfaceType interface = HWInterface::PQ9 );
    void setReceptionHandler( void (*hnd)( unsigned short data ));
    void send( unsigned short data);
};

#endif /* HWINTERFACE_H_ */
