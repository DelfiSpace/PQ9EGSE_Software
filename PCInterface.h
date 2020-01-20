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
#include "DataInterface.h"



class PCInterface: public Task
{
private:
    uint32_t module;
    uint8_t modulePort;
    uint16_t modulePins;
    Queue<unsigned short> rxQueue;
    void (*user_onReceive)( unsigned short data );

    friend void PCInterface_IRQHandler( void );
    friend void taskCallback( void );

protected:
    virtual void run( void );
    virtual void setUp( void );

public:
    PCInterface( void );
    void init( unsigned int baudrate );
    void setReceptionHandler( void (*hnd)( unsigned short data ));
    void send( unsigned short data );
};

#endif /* PCINTERFACE_H_ */
