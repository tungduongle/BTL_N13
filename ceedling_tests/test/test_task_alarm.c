
/*
 * UNIT TEST TASK ALARM - BTL N13 VER3
 *
 * WARNING = 50 PPM
 * DANGER  = 80 PPM
 *
 * Test 1: SAFE
 * Test 2: WARNING
 * Test 3: DANGER
 */

#include "unity.h"

TEST_SOURCE_FILE("-:../app/src/app_ipc.c")

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "mock_bsp_board.h"
#include "app_ipc.h"
#include "task_alarm.h"
#include "app_config.h"

#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>

/* Queue gia lap */
QueueHandle_t xSensorQueue =
    (QueueHandle_t)(uintptr_t)1U;

QueueHandle_t xDisplayQueue =
    (QueueHandle_t)(uintptr_t)2U;

/* Bien Unit Test */
static TaskFunction_t alarmTask;
static jmp_buf stopTask;

static SensorSample_t mauGas;
static AppDisplay_t manHinh;

static unsigned int daDocMau;
static unsigned int daGuiManHinh;

/* ========== GIA LAP FREERTOS ========== */

/* Thoi diem gia lap = 0 tick */
TickType_t xTaskGetTickCount(void)
{
    return 0U;
}

/* Lay dia chi Task_Alarm that */
BaseType_t xTaskCreate(
    TaskFunction_t fn,
    const char *name,
    uint16_t stack,
    void *param,
    UBaseType_t priority,
    TaskHandle_t *handle)
{
    (void)name;
    (void)stack;
    (void)param;
    (void)priority;

    alarmTask = fn;

    if (handle != NULL)
    {
        *handle = (TaskHandle_t)(uintptr_t)3U;
    }

    return pdPASS;
}

/* Gia lap doc du lieu MQ-2 tu Queue */
BaseType_t xQueueReceive(
    QueueHandle_t queue,
    void *item,
    TickType_t wait)
{
    (void)wait;

    TEST_ASSERT_EQUAL_PTR(xSensorQueue, queue);

    if (daDocMau != 0U)
    {
        return pdFALSE;
    }

    *(SensorSample_t *)item = mauGas;
    daDocMau = 1U;

    return pdTRUE;
}

/* Gia lap gui du lieu OLED */
BaseType_t xQueueOverwrite(
    QueueHandle_t queue,
    const void *item)
{
    TEST_ASSERT_EQUAL_PTR(xDisplayQueue, queue);

    manHinh = *(const AppDisplay_t *)item;
    daGuiManHinh++;

    return pdPASS;
}

/* Khong co su kien nut bam */
BaseType_t xTaskNotifyWait(
    uint32_t clearEntry,
    uint32_t clearExit,
    uint32_t *events,
    TickType_t wait)
{
    (void)clearEntry;
    (void)clearExit;
    (void)wait;

    *events = 0U;
    return pdFALSE;
}

BaseType_t xTaskNotify(
    TaskHandle_t handle,
    uint32_t bits,
    eNotifyAction action)
{
    (void)handle;
    (void)bits;
    (void)action;

    return pdPASS;
}

/* Gia lap Timer FreeRTOS */
TimerHandle_t xTimerCreate(
    const char *name,
    TickType_t period,
    BaseType_t reload,
    void *id,
    TimerCallbackFunction_t callback)
{
    (void)name;
    (void)period;
    (void)reload;
    (void)id;
    (void)callback;

    return (TimerHandle_t)(uintptr_t)4U;
}

BaseType_t xTimerReset(
    TimerHandle_t timer,
    TickType_t wait)
{
    (void)timer;
    (void)wait;
    return pdPASS;
}

BaseType_t xTimerStop(
    TimerHandle_t timer,
    TickType_t wait)
{
    (void)timer;
    (void)wait;
    return pdPASS;
}

/* Dung Task sau 1 chu ky */
void vTaskDelayUntil(
    TickType_t *last,
    TickType_t period)
{
    (void)last;

    TEST_ASSERT_EQUAL_UINT32(
        pdMS_TO_TICKS(ALARM_PERIOD_MS),
        period
    );

    longjmp(stopTask, 1);
}

/* ========== CHUAN BI TEST ========== */

void setUp(void)
{
    alarmTask = NULL;

    mauGas.adc = 2000U;
    mauGas.ppm = 0U;

    daDocMau = 0U;
    daGuiManHinh = 0U;
}

void tearDown(void)
{
}

/* Ham kiem tra chung */
static void kiemTraAlarm(
    uint16_t ppm,
    GasStatus_t trangThai,
    uint8_t coi)
{
    /* Gia lap du lieu cam bien */
    mauGas.ppm = ppm;

    /* Su dung nguong mac dinh */
    bsp_flash_load_thresholds_IgnoreAndReturn(0U);

    /* Kiem tra gia tri dieu khien coi */
    bsp_buzzer_write_Expect(coi);

    /* Chay Task_Alarm that */
    TEST_ASSERT_EQUAL(
        pdPASS,
        task_alarm_create()
    );

    TEST_ASSERT_NOT_NULL(alarmTask);

    if (setjmp(stopTask) == 0)
    {
        alarmTask(NULL);
    }

    /* Kiem tra OLED */
    TEST_ASSERT_EQUAL_UINT32(
        1U,
        daGuiManHinh
    );

    TEST_ASSERT_EQUAL(
        trangThai,
        manHinh.status
    );

    TEST_ASSERT_EQUAL_UINT16(
        ppm,
        manHinh.ppm
    );

    TEST_ASSERT_EQUAL_UINT8(
        1U,
        manHinh.sensor_ok
    );
}

/* TEST 1: SAFE */
void test_alarm_safe(void)
{
    /* 20 < 50 PPM */
    kiemTraAlarm(
        20U,
        GAS_SAFE,
        0U
    );
}

/* TEST 2: WARNING */
void test_alarm_warning(void)
{
    /* 50 <= 60 < 80 PPM */
    kiemTraAlarm(
        60U,
        GAS_WARNING,
        1U
    );
}

/* TEST 3: DANGER */
void test_alarm_danger(void)
{
    /* 90 >= 80 PPM */
    kiemTraAlarm(
        90U,
        GAS_DANGER,
        1U
    );
}
