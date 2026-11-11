#include "task_oled.h"
#include "app_ipc.h"
#include "app_config.h"
#include "ssd1306.h"

static void draw_graph(void)
{
    uint16_t history[APP_HISTORY_COUNT];
    uint16_t count;
    uint16_t index;
    uint16_t low = GAS_PPM_MAX;
    uint16_t high = 0U;
    uint16_t range;
    uint16_t ppm;
    uint8_t y0;
    uint8_t y1;

    count = app_history_copy(history, APP_HISTORY_COUNT);
    if (count < 2U)
    {
        return;
    }

    for (index = 0U; index < count; ++index)
    {
        if (history[index] < low)
        {
            low = history[index];
        }
        if (history[index] > high)
        {
            high = history[index];
        }
    }

    /* Auto-scale the graph and enforce a minimum 250-ppm span. */
    range = (uint16_t)(high - low);
    if (range < 250U)
    {
        range = 250U;
    }

    for (index = 1U; index < count; ++index)
    {
        ppm = (uint16_t)(history[index - 1U] - low);
        y0 = (uint8_t)(62U - ((uint32_t)ppm * 22UL) / range);
        ppm = (uint16_t)(history[index] - low);
        y1 = (uint8_t)(62U - ((uint32_t)ppm * 22UL) / range);
        SSD1306_Line(
            (uint8_t)((index - 1U) * 2U),
            y0,
            (uint8_t)(index * 2U),
            y1);
    }
}

static void draw_mute_countdown(uint16_t seconds)
{
    uint16_t value = seconds;
    uint8_t suffixX = 36U;

    while (value >= 10U)
    {
        value = (uint16_t)(value / 10U);
        suffixX = (uint8_t)(suffixX + 6U);
    }

    SSD1306_String(0U, 30U, "MUTE ");
    SSD1306_Number(30U, 30U, seconds);
    SSD1306_String(suffixX, 30U, "S");
}

static void render(const AppDisplay_t *display, uint8_t showMuteEnd)
{
    SSD1306_Clear();

    if (display->setting != SETTING_NONE)
    {
        SSD1306_String(0U, 0U, "SETTING PPM");
        if (display->setting == SETTING_WARNING)
        {
            SSD1306_String(0U, 14U, ">W PPM:");
        }
        else
        {
            SSD1306_String(0U, 14U, " W PPM:");
        }
        SSD1306_Number(60U, 14U, display->warn_threshold);

        if (display->setting == SETTING_DANGER)
        {
            SSD1306_String(0U, 26U, ">D PPM:");
        }
        else
        {
            SSD1306_String(0U, 26U, " D PPM:");
        }
        SSD1306_Number(60U, 26U, display->danger_threshold);
        SSD1306_String(0U, 40U, "PPM:");
        SSD1306_Number(30U, 40U, display->ppm);
        SSD1306_String(66U, 40U, "ADC:");
        SSD1306_Number(90U, 40U, display->adc);

        if (display->flash_error)
        {
            SSD1306_String(0U, 53U, "FLASH ERROR");
        }
        else
        {
            SSD1306_String(0U, 53U, "HOLD SET TO NEXT");
        }
        return;
    }

    if (!display->sensor_ok && display->status != GAS_SENSOR_FAULT)
    {
        SSD1306_String(0U, 0U, "GAS MONITOR");
        SSD1306_String(0U, 18U, "WAIT SENSOR");
        SSD1306_String(0U, 35U, "MQ2 WARMUP");
        return;
    }

    SSD1306_String(0U, 0U, "PPM:");
    if (display->sensor_overrange)
    {
        SSD1306_String(24U, 0U, "OVER");
    }
    else
    {
        SSD1306_Number(24U, 0U, display->ppm);
    }
    SSD1306_String(66U, 0U, "ADC:");
    SSD1306_Number(90U, 0U, display->adc);
    SSD1306_String(0U, 10U, "STATUS:");

    if (display->sensor_overrange)
    {
        SSD1306_String(48U, 10U, "ADC LIMIT");
    }
    else if (display->status == GAS_SAFE)
    {
        SSD1306_String(48U, 10U, "SAFE");
    }
    else if (display->status == GAS_WARNING)
    {
        SSD1306_String(48U, 10U, "WARNING");
    }
    else if (display->status == GAS_DANGER)
    {
        SSD1306_String(48U, 10U, "DANGER");
    }
    else
    {
        SSD1306_String(48U, 10U, "SENSOR FAIL");
    }

    SSD1306_String(0U, 20U, "W:");
    SSD1306_Number(12U, 20U, display->warn_threshold);
    SSD1306_String(62U, 20U, "D:");
    SSD1306_Number(74U, 20U, display->danger_threshold);

    if (display->sensor_overrange)
    {
        SSD1306_String(0U, 30U, "CHECK ADC");
        return;
    }

    if (display->test_active && display->status == GAS_SAFE)
    {
        SSD1306_String(0U, 30U, "TEST BUZZER");
    }
    else if (display->muted || showMuteEnd)
    {
        draw_mute_countdown(display->mute_seconds_remaining);
    }
    else if (display->flash_error)
    {
        SSD1306_String(0U, 30U, "FLASH ERROR");
    }
    else
    {
        SSD1306_String(0U, 30U, "PPM HISTORY");
    }

    draw_graph();
}

static void prvOLEDTask(void *unused)
{
    TickType_t last = xTaskGetTickCount();
    AppDisplay_t data = {0};
    uint8_t initialized = 0U;
    uint8_t previousMuted = 0U;
    GasStatus_t previousStatus = GAS_SAFE;
    uint8_t showMuteEnd;

    (void)unused;

    for (;;)
    {
        if (!initialized)
        {
            initialized = SSD1306_Init();
        }

        if (initialized)
        {
            (void)xQueueReceive(xDisplayQueue, &data, 0U);
            /* Show zero for one successful refresh after normal expiry.
             * A hazard change or recovery must immediately show its new state. */
            showMuteEnd = (previousMuted && !data.muted &&
                           data.mute_seconds_remaining == 0U &&
                           data.status == previousStatus &&
                           (data.status == GAS_WARNING || data.status == GAS_DANGER))
                              ? 1U
                              : 0U;
            render(&data, showMuteEnd);
            if (!SSD1306_UpdateScreen())
            {
                initialized = 0U;
            }
            else
            {
                previousMuted = data.muted;
                previousStatus = data.status;
            }
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(OLED_PERIOD_MS));
    }
}

BaseType_t task_oled_create(void)
{
    return xTaskCreate(
        prvOLEDTask,
        "OLED",
        384U,
        NULL,
        tskIDLE_PRIORITY + 2U,
        NULL);
}
