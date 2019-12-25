/*
 * EGSE.h
 *
 *  Created on: 26 Dec 2019
 *      Author: stefanosperett
 */

#ifndef EGSE_H_
#define EGSE_H_

#include <driverlib.h>
#include "msp.h"
#include "DSerial.h"
#include "DelfiPQcore.h"
#include "PQ9Bus.h"
#include "PQ9Frame.h"
#include "Task.h"
#include "PeriodicTask.h"
#include "TaskManager.h"

#define FCLOCK 48000000

#define EGSE_ADDRESS     8

// callback functions
void periodicTask();

#endif /* EGSE_H_ */
