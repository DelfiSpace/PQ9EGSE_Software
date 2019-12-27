/*
 * PQ9Interface.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef PQ9INTERFACE_H_
#define PQ9INTERFACE_H_

#include <driverlib.h>
#include "HWInterface.h"
#include "DSerial.h"
#include "Queue.h"

class PQ9Interface: public HWInterface
{
private:
    uint32_t module;
    uint8_t modulePort;
    uint16_t modulePins;
    unsigned long TXEnablePort;
    unsigned long TXEnablePin;
    unsigned int baudrate;
    void (*user_onReceive)( unsigned char data );
    Queue rxQueue;
    Queue txQueue;

    friend void PQ9Interface_IRQHandler( void );
    friend void PQ9taskCallback();

protected:
    virtual void executeTask();

public:
    PQ9Interface();
    void init( unsigned int baudrate );
    void setReceptionHandler( void (*hnd)( unsigned char data ));
    void send( unsigned char *data, unsigned short size);
};

#endif /* PQ9INTERFACE_H_ */
