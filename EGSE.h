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
#include "Console.h"
#include "DelfiPQcore.h"
#include "Task.h"
#include "PeriodicTask.h"
#include "TaskManager.h"
#include "PCInterface.h"
#include "HWInterface.h"
#include "PeriodicTaskNotifier.h"

#define FCLOCK 48000000

// callback functions
void periodicTask();

#endif /* EGSE_H_ */
