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

#define HLDLC_START_FLAG             0x7E
#define HLDLC_CONTROL_FLAG           0x7D
#define HLDLC_STOP_FLAG              0x7C
#define HLDLC_ESCAPE_START_FLAG      0x5E
#define HLDLC_ESCAPE_CONTROL_FLAG    0x5D
#define HLDLC_ESCAPE_STOP_FLAG       0x5C

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
    virtual void executeTask();

public:
    PCInterface();
    void init(unsigned int baudrate);
    void setReceptionHandler( void (*hnd)( unsigned short data ));
    void send( unsigned short data);
};

#endif /* PCINTERFACE_H_ */
