/*
 * DataInterface.h
 *
 *  Created on: 7 Jan 2020
 *      Author: stefanosperett
 */

#ifndef DATAINTERFACE_H_
#define DATAINTERFACE_H_

#define FIRST_BYTE                   0x80

#define ADDRESS_BIT                  0x4000
#define STOP_TRANSMISSION            0x2000
#define COMMAND                      0x1000
#define INITIALIZE                   0x00
#define INTERFACE_PQ9                0x01
#define INTERFACE_RS485              0x02

#define RESET_EGSE                   0x8000

#endif /* DATAINTERFACE_H_ */
