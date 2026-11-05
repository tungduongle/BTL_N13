
#ifndef APP_CFG_H
#define APP_CFG_H

/* Chu ky cac task (ms) */
#define SENSOR_PERIOD_MS           100U
#define ALARM_PERIOD_MS            20U
#define OLED_PERIOD_MS             200U

/* Thoi gian Software Timer (ms) */
#define MUTE_DURATION_MS           30000U
#define TEST_DURATION_MS           1000U

/* Cau hinh Queue */
#define SENSOR_QUEUE_LENGTH        4U
#define DISPLAY_QUEUE_LENGTH      1U

/* Nguong GAS mac dinh - can hieu chuan MQ-2 */
#define DEFAULT_WARNING_THRESHOLD 1500U
#define DEFAULT_DANGER_THRESHOLD  2500U

/* Buoc thay doi nguong */
#define THRESHOLD_STEP             50U

/* Thoi gian giu nut de xac dinh nhan dai */
#define BUTTON_LONG_PRESS_MS       2000U

/* ADC 12-bit cua STM32F103 */
#define ADC_MAX_VALUE              4095U

#endif
