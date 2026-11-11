#ifndef APP_IPC_H
#define APP_IPC_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* Model voltage reaches/exceeds Vc; not ppm. ADC 4095 alone is numeric. */
#define SENSOR_ADC_OVERRANGE 0xFFFEU

typedef enum
{
    GAS_SAFE = 0,
    GAS_WARNING = 1,
    GAS_DANGER = 2,
    GAS_SENSOR_FAULT = 3
} GasStatus_t;

typedef enum
{
    SETTING_NONE = 0,
    SETTING_WARNING = 1,
    SETTING_DANGER = 2
} SettingStep_t;

typedef struct
{
    uint16_t ppm; /* LPG ppm or SENSOR_ADC_OVERRANGE. */
    uint16_t adc; /* ADC value used for this conversion, after filtering. */
} SensorSample_t;

typedef struct
{
    uint16_t ppm;
    uint16_t warn_threshold;   /* Estimated LPG ppm. */
    uint16_t danger_threshold; /* Estimated LPG ppm. */
    uint16_t mute_seconds_remaining;
    GasStatus_t status;
    SettingStep_t setting;
    uint8_t sensor_ok;
    uint8_t sensor_overrange;
    uint8_t muted;
    uint8_t test_active;
    uint8_t flash_error;
    uint16_t adc;
} AppDisplay_t;

/* SensorSample_t, queue length 1: paired PPM and ADC. */
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xDisplayQueue;
extern SemaphoreHandle_t xButtonSemaphore;

uint8_t app_ipc_init(void);
void app_history_push(uint16_t ppm);
uint16_t app_history_copy(uint16_t *out, uint16_t capacity);

/* Event bits sent to the alarm task using direct-to-task notifications. */
#define BTN_ALARM_SHORT  (1UL << 0)
#define BTN_ALARM_LONG   (1UL << 1)
#define BTN_SET_SHORT    (1UL << 2)
#define BTN_SET_LONG     (1UL << 3)
#define EVT_MUTE_TIMEOUT (1UL << 4)
#define EVT_TEST_TIMEOUT (1UL << 5)
#define BTN_RESET_DEFAULTS (1UL << 6) /* Hold ALARM + SET together. */

#endif
