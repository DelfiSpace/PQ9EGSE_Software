/*
 * Queue.cpp
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#include <Queue.h>

Queue::Queue()
{
    // initialize the circular buffer
    tail = 0;
    head = 0;
    c = 0;
}

bool Queue::push( unsigned char data )
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

bool Queue::pop( unsigned char &data)
{
    if (empty())
    {
        return false;
    }
    data = buffer[tail];
    tail = (tail + 1) % SIZE;
    return true;
}

bool Queue::empty()
{
    return head == tail;
}

bool Queue::full()
{
    return tail == (head + 1) % SIZE;
}

unsigned int Queue::size()
{
    return (head - tail) % SIZE;
}

