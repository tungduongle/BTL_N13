
/*
 * UNIT TEST MQ-2 - BTL N13 VER3
 *
 * Test 1: Trung binh 8 mau ADC
 * Test 2: ADC = 0 khong hop le
 * Test 3: Loc nhieu IIR
 */

#include "unity.h"

TEST_SOURCE_FILE("-:../app/src/app_ipc.c")

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "mock_bsp_board.h"
#include "app_ipc.h"
#include "task_sensor.h"
#include "app_config.h"

#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>

/* Queue gia lap */
QueueHandle_t xSensorQueue =
    (QueueHandle_t)(uintptr_t)1U;

/* Bien dung trong Unit Test */
static TaskFunction_t sensorTask;
static jmp_buf stopTask;

static unsigned int soChuKy;
static unsigned int daChay;
static unsigned int soLanGui;
static unsigned int soLanLuu;

static SensorSample_t ketQua[2];
static uint16_t ppmDaLuu;

/* ========== GIA LAP FREERTOS ========== */

TickType_t xTaskGetTickCount(void)
{
    return 0U;
}

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
    (void)handle;

    sensorTask = fn;
    return pdPASS;
}

/* Gia lap gui du lieu vao Queue */
BaseType_t xQueueOverwrite(
    QueueHandle_t queue,
    const void *item)
{
    TEST_ASSERT_EQUAL_PTR(xSensorQueue, queue);
    TEST_ASSERT_TRUE(soLanGui < 2U);

    ketQua[soLanGui++] =
        *(const SensorSample_t *)item;

    return pdPASS;
}

/* Gia lap luu lich su PPM */
void app_history_push(uint16_t ppm)
{
    soLanLuu++;
    ppmDaLuu = ppm;
}

/* Dung Task sau so chu ky yeu cau */
void vTaskDelayUntil(
    TickType_t *last,
    TickType_t period)
{
    (void)last;

    TEST_ASSERT_EQUAL_UINT32(
        pdMS_TO_TICKS(SENSOR_PERIOD_MS),
        period
    );

    daChay++;

    if (daChay >= soChuKy)
    {
        longjmp(stopTask, 1);
    }
}

/* ========== CHUAN BI TEST ========== */

void setUp(void)
{
    sensorTask = NULL;
    soChuKy = 0U;
    daChay = 0U;
    soLanGui = 0U;
    soLanLuu = 0U;
    ppmDaLuu = 0U;
}

void tearDown(void)
{
}

/* Gia lap 8 lan doc ADC */
static void doc8Mau(uint16_t adc)
{
    unsigned int i;

    for (i = 0U; i < 8U; i++)
    {
        bsp_adc_read_ExpectAndReturn(adc);
    }
}

/* Chay Task_Sensor that */
static void chaySensor(unsigned int n)
{
    soChuKy = n;

    TEST_ASSERT_EQUAL(
        pdPASS,
        task_sensor_create()
    );

    TEST_ASSERT_NOT_NULL(sensorTask);

    if (setjmp(stopTask) == 0)
    {
        sensorTask(NULL);
    }

    TEST_ASSERT_EQUAL_UINT32(n, daChay);
}

/* TEST 1: Trung binh 8 mau ADC */
void test_mq2_trung_binh_adc(void)
{
    unsigned int i;

    /* 4 mau = 1000 */
    for (i = 0U; i < 4U; i++)
    {
        bsp_adc_read_ExpectAndReturn(1000U);
    }

    /* 4 mau = 2000 */
    for (i = 0U; i < 4U; i++)
    {
        bsp_adc_read_ExpectAndReturn(2000U);
    }

    chaySensor(1U);

    TEST_ASSERT_EQUAL_UINT32(1U, soLanGui);

    /* Trung binh = 1500 */
    TEST_ASSERT_EQUAL_UINT16(
        1500U,
        ketQua[0].adc
    );

    TEST_ASSERT_EQUAL_UINT32(1U, soLanLuu);
    TEST_ASSERT_EQUAL_UINT16(ketQua[0].ppm, ppmDaLuu);
}

/* TEST 2: ADC = 0 khong hop le */
void test_mq2_adc_loi(void)
{
    bsp_adc_read_ExpectAndReturn(0U);

    chaySensor(1U);

    /* Du lieu loi khong duoc gui */
    TEST_ASSERT_EQUAL_UINT32(0U, soLanGui);
    TEST_ASSERT_EQUAL_UINT32(0U, soLanLuu);
}

/* TEST 3: Kiem tra bo loc IIR */
void test_mq2_loc_nhieu_iir(void)
{
    /* Chu ky 1: ADC = 1000 */
    doc8Mau(1000U);

    /* Chu ky 2: ADC = 2000 */
    doc8Mau(2000U);

    chaySensor(2U);

    TEST_ASSERT_EQUAL_UINT32(2U, soLanGui);

    TEST_ASSERT_EQUAL_UINT16(
        1000U,
        ketQua[0].adc
    );

    /* IIR = (3*1000 + 2000 + 2)/4 = 1250 */
    TEST_ASSERT_EQUAL_UINT16(
        1250U,
        ketQua[1].adc
    );
}
