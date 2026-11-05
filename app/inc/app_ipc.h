#ifndef APP_IPC_H
#define APP_IPC_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "app_types.h"
#include "app_cfg.h"

extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xDisplayQueue;
extern SemaphoreHandle_t xButtonSemaphore;
extern TaskHandle_t xAlarmTaskHandle;
extern TimerHandle_t xMuteTimer;
extern TimerHandle_t xTestTimer;

#define APP_NOTIFY_ALARM_BUTTON    (1UL << 0)
#define APP_NOTIFY_SET_SHORT       (1UL << 1)
#define APP_NOTIFY_SET_LONG        (1UL << 2)
#define APP_NOTIFY_MUTE_TIMEOUT    (1UL << 3)
#define APP_NOTIFY_TEST_TIMEOUT    (1UL << 4)

BaseType_t app_ipc_init(void);

#endif
