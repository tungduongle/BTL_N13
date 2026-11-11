#include "task_alarm.h"
#include "app_ipc.h"
#include "app_config.h"
#include "bsp_board.h"
#include "timers.h"

static TaskHandle_t alarmTaskHandle = NULL;
static TimerHandle_t muteTimer = NULL;
static TimerHandle_t testTimer = NULL;

void task_alarm_notify(uint32_t eventBits)
{
    if (alarmTaskHandle != NULL)
    {
        (void)xTaskNotify(alarmTaskHandle, eventBits, eSetBits);
    }
}

static void mute_timeout_cb(TimerHandle_t timer)
{
    (void)timer;
    task_alarm_notify(EVT_MUTE_TIMEOUT);
}

static void test_timeout_cb(TimerHandle_t timer)
{
    (void)timer;
    task_alarm_notify(EVT_TEST_TIMEOUT);
}

static GasStatus_t classify(
    uint16_t ppm,
    GasStatus_t previous,
    uint16_t warning,
    uint16_t danger)
{
    if (previous == GAS_DANGER)
    {
        if ((uint32_t)ppm + GAS_HYSTERESIS_PPM >= danger)
        {
            return GAS_DANGER;
        }
        if ((uint32_t)ppm + GAS_HYSTERESIS_PPM >= warning)
        {
            return GAS_WARNING;
        }
        return GAS_SAFE;
    }

    if (previous == GAS_WARNING)
    {
        if (ppm >= danger)
        {
            return GAS_DANGER;
        }
        if ((uint32_t)ppm + GAS_HYSTERESIS_PPM >= warning)
        {
            return GAS_WARNING;
        }
        return GAS_SAFE;
    }

    if (ppm >= danger)
    {
        return GAS_DANGER;
    }
    if (ppm >= warning)
    {
        return GAS_WARNING;
    }
    return GAS_SAFE;
}

static void stop_mute(uint8_t *muted)
{
    if (*muted != 0U)
    {
        *muted = 0U;
        /* Local state cancels mute even if the timer command queue is full.
         * Deadline checks below reject callbacks from a canceled run. */
        (void)xTimerStop(muteTimer, 0U);
    }
}

static void prvAlarmTask(void *unused)
{
    AppDisplay_t ui = {0};
    SensorSample_t sample;
    uint16_t newWarn;
    uint16_t newDanger;
    uint32_t events;
    uint8_t seenSensor = 0U;
    uint8_t testActive = 0U;
    GasStatus_t previous;
    TickType_t last = xTaskGetTickCount();
    TickType_t lastSensor = last;
    TickType_t muteStartedAt = last;
    TickType_t testStartedAt = last;
    TickType_t now;
    TickType_t elapsed;
    TickType_t remaining;
    uint32_t phase;
    uint32_t ms;

    (void)unused;

    ui.warn_threshold = GAS_WARNING_PPM_DEFAULT;
    ui.danger_threshold = GAS_DANGER_PPM_DEFAULT;
    (void)bsp_flash_load_thresholds(&ui.warn_threshold, &ui.danger_threshold);
    ui.status = GAS_SAFE;
    ui.setting = SETTING_NONE;

    for (;;)
    {
        now = xTaskGetTickCount();
        while (xQueueReceive(xSensorQueue, &sample, 0U) == pdTRUE)
        {
            ui.adc = sample.adc;
            ui.sensor_overrange = (sample.ppm == SENSOR_ADC_OVERRANGE) ? 1U : 0U;
            if (!ui.sensor_overrange)
            {
                ui.ppm = sample.ppm;
            }
            seenSensor = 1U;
            lastSensor = now;
        }

        if ((now - lastSensor) > pdMS_TO_TICKS(SENSOR_TIMEOUT_MS))
        {
            /* A stale limit indication becomes a general sensor-data fault. */
            ui.sensor_overrange = 0U;
        }
        ui.sensor_ok = (seenSensor != 0U &&
                        ui.sensor_overrange == 0U &&
                        (now - lastSensor) <= pdMS_TO_TICKS(SENSOR_TIMEOUT_MS))
                           ? 1U
                           : 0U;
        previous = ui.status;

        if (ui.sensor_overrange ||
            (!ui.sensor_ok && (now - lastSensor) > pdMS_TO_TICKS(SENSOR_TIMEOUT_MS)))
        {
            ui.status = GAS_SENSOR_FAULT;
            testActive = 0U;
        }
        else if (ui.sensor_ok)
        {
            ui.status = classify(
                ui.ppm,
                previous,
                ui.warn_threshold,
                ui.danger_threshold);
        }

        /* A new hazard cancels mute and threshold editing. */
        if ((ui.status == GAS_DANGER && previous != GAS_DANGER) ||
            (ui.status == GAS_WARNING && previous == GAS_SAFE) ||
            ui.status == GAS_SENSOR_FAULT)
        {
            stop_mute(&ui.muted);
        }
        if (ui.status != GAS_SAFE && ui.setting != SETTING_NONE)
        {
            ui.setting = SETTING_NONE;
            ui.flash_error = 0U;
        }
        if (ui.status == GAS_SAFE)
        {
            stop_mute(&ui.muted);
        }

        events = 0UL;
        (void)xTaskNotifyWait(0UL, 0xFFFFFFFFUL, &events, 0U);
        if (events & BTN_RESET_DEFAULTS)
        {
            /* A chord must not also adjust settings, mute or test the buzzer. */
            events &= ~(BTN_ALARM_SHORT | BTN_ALARM_LONG |
                        BTN_SET_SHORT | BTN_SET_LONG);
            if (ui.status == GAS_SAFE && ui.sensor_ok != 0U)
            {
                if (bsp_flash_save_thresholds(
                        GAS_WARNING_PPM_DEFAULT, GAS_DANGER_PPM_DEFAULT))
                {
                    ui.warn_threshold = GAS_WARNING_PPM_DEFAULT;
                    ui.danger_threshold = GAS_DANGER_PPM_DEFAULT;
                    newWarn = ui.warn_threshold;
                    newDanger = ui.danger_threshold;
                    ui.setting = SETTING_NONE;
                    ui.flash_error = 0U;
                    testActive = 0U;
                    /* Lower defaults can make the current sample hazardous. */
                    ui.status = classify(ui.ppm, GAS_SAFE,
                                         ui.warn_threshold, ui.danger_threshold);
                }
                else
                {
                    /* Keep the running thresholds and any unsaved edits. */
                    ui.flash_error = 1U;
                }
            }
        }
        if ((events & BTN_ALARM_LONG) &&
            ui.status == GAS_SAFE &&
            ui.setting == SETTING_NONE)
        {
            if (xTimerReset(testTimer, 0U) == pdPASS)
            {
                testStartedAt = xTaskGetTickCount();
                testActive = 1U;
            }
        }

        if (events & BTN_SET_LONG)
        {
            if (ui.status == GAS_SAFE)
            {
                if (ui.setting == SETTING_NONE)
                {
                    newWarn = ui.warn_threshold;
                    newDanger = ui.danger_threshold;
                    ui.setting = SETTING_WARNING;
                    ui.flash_error = 0U;
                }
                else if (ui.setting == SETTING_WARNING)
                {
                    ui.setting = SETTING_DANGER;
                }
                else /* End setting: save the new values if they are valid. */
                {
                    if (bsp_flash_save_thresholds(newWarn, newDanger))
                    {
                        ui.warn_threshold = newWarn;
                        ui.danger_threshold = newDanger;
                        ui.setting = SETTING_NONE;
                        ui.flash_error = 0U;
                    }
                    else
                    {
                        ui.flash_error = 1U;
                    }
                }
            }
        }

        /* Short SET raises the threshold currently being edited. */
        if (events & BTN_SET_SHORT)
        {
            if (ui.status == GAS_SAFE && ui.setting != SETTING_NONE)
            {
                if (ui.setting == SETTING_WARNING)
                {
                    if ((uint32_t)newWarn + GAS_THRESHOLD_STEP_PPM + GAS_MIN_GAP_PPM <=
                        newDanger)
                    {
                        newWarn += GAS_THRESHOLD_STEP_PPM;
                    }
                }
                else if (ui.setting == SETTING_DANGER)
                {
                    if ((uint32_t)newDanger + GAS_THRESHOLD_STEP_PPM <= GAS_PPM_MAX)
                    {
                        newDanger += GAS_THRESHOLD_STEP_PPM;
                    }
                }
            }
        }

        if (events & BTN_ALARM_SHORT)
        {
            /* Short ALARM lowers thresholds while the system is in setting mode. */
            if (ui.setting != SETTING_NONE && ui.status == GAS_SAFE)
            {
                if (ui.setting == SETTING_WARNING)
                {
                    if (newWarn >= (GAS_THRESHOLD_MIN_PPM + GAS_THRESHOLD_STEP_PPM))
                    {
                        newWarn -= GAS_THRESHOLD_STEP_PPM;
                    }
                }
                else if (ui.setting == SETTING_DANGER)
                {
                    if ((uint32_t)newDanger >=
                        (uint32_t)newWarn + GAS_MIN_GAP_PPM + GAS_THRESHOLD_STEP_PPM)
                    {
                        newDanger -= GAS_THRESHOLD_STEP_PPM;
                    }
                }
            }
            /* Outside setting mode, short ALARM mutes an active warning. */
            else if ((ui.status == GAS_WARNING || ui.status == GAS_DANGER) &&
                     ui.setting == SETTING_NONE)
            {
                if (xTimerReset(muteTimer, 0U) == pdPASS)
                {
                    muteStartedAt = xTaskGetTickCount();
                    ui.muted = 1U;
                }
            }
        }

        /* Publish the actual deadline, including a new mute from this cycle.
         * Old or delayed callbacks cannot expire a newer timer run. */
        now = xTaskGetTickCount();
        ui.mute_seconds_remaining = 0U;
        if (ui.muted != 0U)
        {
            elapsed = (TickType_t)(now - muteStartedAt);
            if (elapsed >= pdMS_TO_TICKS(MUTE_TIME_MS))
            {
                ui.muted = 0U;
            }
            else
            {
                remaining = pdMS_TO_TICKS(MUTE_TIME_MS) - elapsed;
                /* Round up: display 1 second until the deadline is reached. */
                ui.mute_seconds_remaining = (uint16_t)(
                    1U + (remaining - 1U) / configTICK_RATE_HZ);
            }
        }
        if (testActive != 0U &&
            (TickType_t)(now - testStartedAt) >= pdMS_TO_TICKS(TEST_TIME_MS))
        {
            testActive = 0U;
        }
        ui.test_active = (testActive != 0U && ui.sensor_ok != 0U &&
                          ui.status == GAS_SAFE && ui.setting == SETTING_NONE)
                             ? 1U
                             : 0U;

        /* Show edited values, but save them only after long SET. */
        {
            AppDisplay_t snapshot = ui;

            if (ui.setting != SETTING_NONE)
            {
                snapshot.warn_threshold = newWarn;
                snapshot.danger_threshold = newDanger;
            }
            (void)xQueueOverwrite(xDisplayQueue, &snapshot);
        }

        if (!seenSensor && ui.status == GAS_SAFE)
        {
            bsp_buzzer_write(0U); /* Sensor warm-up: SAFE is not confirmed yet. */
        }
        else if (ui.status == GAS_SENSOR_FAULT)
        {
            bsp_buzzer_write(1U); /* Continuous indication of sensor loss. */
        }
        else if (ui.status == GAS_SAFE)
        {
            bsp_buzzer_write(ui.test_active);
        }
        else if (ui.muted)
        {
            bsp_buzzer_write(0U);
        }
        else
        {
            /* Slow: 500 ms ON/OFF; fast: 100 ms ON/OFF. */
            ms = (uint32_t)now * (uint32_t)portTICK_PERIOD_MS;
            phase = (ui.status == GAS_DANGER) ? 200UL : 1000UL;
            bsp_buzzer_write((ms % phase) < (phase / 2UL) ? 1U : 0U);
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(ALARM_PERIOD_MS));
    }
}

BaseType_t task_alarm_create(void)
{
    muteTimer = xTimerCreate(
        "MUTE",
        pdMS_TO_TICKS(MUTE_TIME_MS),
        pdFALSE,
        NULL,
        mute_timeout_cb);
    testTimer = xTimerCreate(
        "TEST",
        pdMS_TO_TICKS(TEST_TIME_MS),
        pdFALSE,
        NULL,
        test_timeout_cb);

    if (muteTimer == NULL || testTimer == NULL)
    {
        return pdFAIL;
    }

    return xTaskCreate(
        prvAlarmTask,
        "ALARM",
        256U,
        NULL,
        tskIDLE_PRIORITY + 4U,
        &alarmTaskHandle);
}
