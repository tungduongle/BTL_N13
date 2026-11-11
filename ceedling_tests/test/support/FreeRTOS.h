
#ifndef TEST_FREERTOS_H
#define TEST_FREERTOS_H

#include <stdint.h>
#include <stddef.h>

/* Tan so tick FreeRTOS: 1000 Hz */
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ 1000U
#endif

/* Kieu du lieu FreeRTOS gia lap */
typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t TickType_t;

typedef void *TaskHandle_t;
typedef void *QueueHandle_t;
typedef void *SemaphoreHandle_t;
typedef void *TimerHandle_t;

typedef void (*TaskFunction_t)(void *);
typedef void (*TimerCallbackFunction_t)(TimerHandle_t);

typedef enum {
    eSetBits = 1
} eNotifyAction;

/* Gia tri tra ve */
#define pdTRUE  1
#define pdFALSE 0
#define pdPASS  1
#define pdFAIL  0

#define tskIDLE_PRIORITY 0U
#define portTICK_PERIOD_MS 1U
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

/* FreeRTOS API prototypes */
TickType_t xTaskGetTickCount(void);

BaseType_t xTaskCreate(
    TaskFunction_t fn,
    const char *name,
    uint16_t stack,
    void *param,
    UBaseType_t priority,
    TaskHandle_t *handle
);

BaseType_t xTaskNotify(
    TaskHandle_t handle,
    uint32_t bits,
    eNotifyAction action
);

BaseType_t xTaskNotifyWait(
    uint32_t clearEntry,
    uint32_t clearExit,
    uint32_t *events,
    TickType_t wait
);

void vTaskDelayUntil(
    TickType_t *last,
    TickType_t period
);

BaseType_t xQueueReceive(
    QueueHandle_t queue,
    void *data,
    TickType_t wait
);

BaseType_t xQueueOverwrite(
    QueueHandle_t queue,
    const void *data
);

TimerHandle_t xTimerCreate(
    const char *name,
    TickType_t period,
    BaseType_t autoReload,
    void *id,
    TimerCallbackFunction_t callback
);

BaseType_t xTimerReset(
    TimerHandle_t timer,
    TickType_t wait
);

BaseType_t xTimerStop(
    TimerHandle_t timer,
    TickType_t wait
);

#endif
