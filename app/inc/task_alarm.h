#ifndef TASK_ALARM_H
#define TASK_ALARM_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

BaseType_t task_alarm_create(void);
void task_alarm_notify(uint32_t eventBits);

#endif
