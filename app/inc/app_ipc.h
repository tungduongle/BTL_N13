
#ifndef APP_IPC_H
#define APP_IPC_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "app_types.h"
#include "app_cfg.h"

/* Queue giao tiep giua cac task */
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xDisplayQueue;

/* Semaphore dong bo su kien nut nhan */
extern SemaphoreHandle_t xButtonSemaphore;

/* Task handle cua Alarm */
extern TaskHandle_t xAlarmTaskHandle;

/* Software Timer */
extern TimerHandle_t xMuteTimer;
extern TimerHandle_t xTestTimer;

/* Notification bits tu Button sang Alarm */
#define APP_NOTIFY_ALARM_BUTTON    (1UL << 0)
#define APP_NOTIFY_SET_SHORT       (1UL << 1)
#define APP_NOTIFY_SET_LONG        (1UL << 2)

/* Khoi tao cac doi tuong IPC */
BaseType_t app_ipc_init(void);

#endif
