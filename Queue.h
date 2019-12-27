/*
 * Queue.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#define SIZE 4096

template <class T>
class Queue
{
private:
    T buffer[SIZE];
    unsigned short head;
    unsigned short tail;
    unsigned short c;

public:
    Queue()
    {
        // initialize the circular buffer
        tail = 0;
        head = 0;
        c = 0;
    }

    bool push( T data )
    {
        if (full())
        {
            return false;
        }
        buffer[head] = data;
        head = (head + 1) % SIZE;
        c++;
        return true;
    }

    bool pop( T &data )
    {
        if (empty())
        {
            return false;
        }
        data = buffer[tail];
        tail = (tail + 1) % SIZE;
        return true;
    }

    bool empty()
    {
        return head == tail;
    }

    bool full()
    {
        return tail == (head + 1) % SIZE;
    }

    unsigned int size()
    {
        return (head - tail) % SIZE;
    }

};

#endif /* QUEUE_H_ */
