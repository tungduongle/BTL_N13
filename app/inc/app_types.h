
#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>

/* Muc canh bao khi phat hien khi GAS */
typedef enum {
    GAS_LEVEL_SAFE = 0,
    GAS_LEVEL_WARNING,
    GAS_LEVEL_DANGER
} GasLevel_t;

/* Trang thai hoat dong cua he thong */
typedef enum {
    APP_MODE_NORMAL = 0,
    APP_MODE_SETTING
} AppMode_t;

/* Du lieu tu Sensor gui sang Alarm */
typedef struct {
    uint16_t adc_raw;
    uint32_t sample_tick;
} SensorData_t;

/* Du lieu tu Alarm gui sang OLED */
typedef struct {
    uint16_t adc_raw;
    uint16_t warning_threshold;
    uint16_t danger_threshold;
    GasLevel_t gas_level;
    AppMode_t app_mode;
    uint8_t alarm_active;
    uint8_t is_muted;
    uint8_t test_active;
} DisplayData_t;

#endif
