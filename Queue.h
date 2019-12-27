/*
 * Queue.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#define SIZE 4096

class Queue
{
private:
    unsigned char buffer[SIZE];
    unsigned short head;
    unsigned short tail;
    unsigned short c;
public:
    Queue();
    bool push( unsigned char data );
    bool pop( unsigned char &data );
    bool empty();
    bool full();
    unsigned int size();

};

#endif /* QUEUE_H_ */
