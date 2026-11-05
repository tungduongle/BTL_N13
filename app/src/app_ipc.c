#include "app_ipc.h"

QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xDisplayQueue = NULL;
SemaphoreHandle_t xButtonSemaphore = NULL;
TaskHandle_t xAlarmTaskHandle = NULL;
TimerHandle_t xMuteTimer = NULL;
TimerHandle_t xTestTimer = NULL;

static void prvMuteTimerCallback(TimerHandle_t xTimer);
static void prvTestTimerCallback(TimerHandle_t xTimer);

static void prvMuteTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    if (xAlarmTaskHandle != NULL) {
        (void)xTaskNotify(xAlarmTaskHandle, APP_NOTIFY_MUTE_TIMEOUT, eSetBits);
    }
}

static void prvTestTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    if (xAlarmTaskHandle != NULL) {
        (void)xTaskNotify(xAlarmTaskHandle, APP_NOTIFY_TEST_TIMEOUT, eSetBits);
    }
}

BaseType_t app_ipc_init(void)
{
    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorData_t));
    if (xSensorQueue == NULL) return pdFAIL;

    xDisplayQueue = xQueueCreate(DISPLAY_QUEUE_LENGTH, sizeof(DisplayData_t));
    if (xDisplayQueue == NULL) return pdFAIL;

    xButtonSemaphore = xSemaphoreCreateBinary();
    if (xButtonSemaphore == NULL) return pdFAIL;

    xMuteTimer = xTimerCreate("MuteTimer", pdMS_TO_TICKS(MUTE_DURATION_MS), pdFALSE, NULL, prvMuteTimerCallback);
    if (xMuteTimer == NULL) return pdFAIL;

    xTestTimer = xTimerCreate("TestTimer", pdMS_TO_TICKS(TEST_DURATION_MS), pdFALSE, NULL, prvTestTimerCallback);
    if (xTestTimer == NULL) return pdFAIL;

    return pdPASS;
}
