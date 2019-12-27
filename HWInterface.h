/*
 * HWInterface.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef HWINTERFACE_H_
#define HWINTERFACE_H_

#include "Task.h"

class HWInterface : public Task
{
public:
    HWInterface( void (*function)( void ) ) : Task(function) {};
    virtual ~HWInterface( ) {};
    virtual void init( unsigned int baudrate ) = 0;
    virtual void setReceptionHandler( void (*hnd)( unsigned char data )) = 0;
    virtual void send( unsigned char *data, unsigned short size) = 0;
};

#endif /* HWINTERFACE_H_ */
