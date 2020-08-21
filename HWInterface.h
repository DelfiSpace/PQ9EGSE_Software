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
#include "Console.h"
#include "Queue.h"
#include "DataInterface.h"

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
    friend void PQ9taskCallback( void );

    bool toggleCharge = false;
    bool toggleDischarge = false;

protected:
    virtual void run( void );

public:
    enum InterfaceType {PQ9, RS485};
    HWInterface( void );
    void init( InterfaceType interface = HWInterface::PQ9 );
    void setReceptionHandler( void (*hnd)( unsigned short data ));
    void send( unsigned short data );
};

#endif /* HWINTERFACE_H_ */
