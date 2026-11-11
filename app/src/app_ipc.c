#include "app_ipc.h"
#include "app_config.h"
#include "task.h"

QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xDisplayQueue = NULL;
SemaphoreHandle_t xButtonSemaphore = NULL;

static SemaphoreHandle_t xHistoryMutex = NULL;
static uint16_t history[APP_HISTORY_COUNT];
static uint8_t historyHead = 0U;
static uint8_t historyCount = 0U;

uint8_t app_ipc_init(void)
{
    xSensorQueue = xQueueCreate(1U, sizeof(SensorSample_t));
    xDisplayQueue = xQueueCreate(1U, sizeof(AppDisplay_t));
    xButtonSemaphore = xSemaphoreCreateBinary();
    xHistoryMutex = xSemaphoreCreateMutex();

    return (xSensorQueue != NULL &&
            xDisplayQueue != NULL &&
            xButtonSemaphore != NULL &&
            xHistoryMutex != NULL) ? 1U : 0U;
}

void app_history_push(uint16_t ppm)
{
    if (xSemaphoreTake(xHistoryMutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        history[historyHead] = ppm;
        historyHead = (uint8_t)((historyHead + 1U) % APP_HISTORY_COUNT);

        if (historyCount < APP_HISTORY_COUNT)
        {
            ++historyCount;
        }

        xSemaphoreGive(xHistoryMutex);
    }
}

uint16_t app_history_copy(uint16_t *out, uint16_t capacity)
{
    uint16_t index;
    uint16_t count;
    uint16_t start;

    if (out == NULL || capacity == 0U)
    {
        return 0U;
    }
    if (xSemaphoreTake(xHistoryMutex, pdMS_TO_TICKS(5)) != pdTRUE)
    {
        return 0U;
    }

    count = historyCount;
    if (count > capacity)
    {
        count = capacity;
    }
    start = (uint16_t)((historyHead + APP_HISTORY_COUNT - count) % APP_HISTORY_COUNT);

    for (index = 0U; index < count; ++index)
    {
        out[index] = history[(start + index) % APP_HISTORY_COUNT];
    }

    xSemaphoreGive(xHistoryMutex);
    return count;
}
