#include "task_button.h"
#include "task_alarm.h"
#include "app_ipc.h"
#include "bsp_board.h"
#include "app_config.h"
#include "hal_gpio.h"

/* EXTI12 and EXTI13 share the STM32F1 EXTI15_10 interrupt vector. */
void EXTI15_10_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;
    uint32_t pending = HAL_GPIO_ButtonIRQPending();

    HAL_GPIO_ClearButtonIRQ(pending);
    if (pending != 0UL && xButtonSemaphore != NULL)
    {
        xSemaphoreGiveFromISR(xButtonSemaphore, &woken);
    }

    portYIELD_FROM_ISR(woken);
}

typedef struct
{
    uint8_t raw;
    uint8_t stable;
    uint8_t longSent;
    TickType_t changeAt;
    TickType_t pressedAt;
} ButtonFilter_t;

static uint32_t update_button(
    ButtonFilter_t *button,
    uint8_t raw,
    TickType_t now,
    uint32_t shortEvent,
    uint32_t longEvent)
{
    uint32_t event = 0UL;

    if (raw != button->raw)
    {
        button->raw = raw;
        button->changeAt = now;
    }

    if (button->stable != button->raw &&
        (now - button->changeAt) >= pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS))
    {
        button->stable = button->raw;
        if (button->stable != 0U)
        {
            button->pressedAt = now;
            button->longSent = 0U;
        }
        else if (button->longSent == 0U)
        {
            event = shortEvent;
        }
    }

    if (button->stable != 0U && button->longSent == 0U &&
        (now - button->pressedAt) >= pdMS_TO_TICKS(BUTTON_LONG_MS))
    {
        button->longSent = 1U;
        event = longEvent;
    }

    return event;
}

static void prvButtonTask(void *unused)
{
    ButtonFilter_t alarm = {0};
    ButtonFilter_t set = {0};
    TickType_t now;
    TickType_t comboStartedAt = 0U;
    uint16_t input;
    uint32_t events;
    uint8_t comboActive = 0U;
    uint8_t comboHeld = 0U;
    uint8_t comboSent = 0U;

    (void)unused;
    bsp_buttons_irq_enable();
    (void)xSemaphoreGive(xButtonSemaphore); /* Sample inputs once at boot. */

    for (;;)
    {
        /* Wake on EXTI; poll every 20 ms while held or debouncing. */
        if (alarm.stable || set.stable || alarm.raw || set.raw)
        {
            (void)xSemaphoreTake(xButtonSemaphore, pdMS_TO_TICKS(20));
        }
        else
        {
            (void)xSemaphoreTake(xButtonSemaphore, portMAX_DELAY);
        }

        now = xTaskGetTickCount();
        input = bsp_buttons_pressed_mask();
        events = update_button(
            &alarm,
            (input & BSP_BUTTON_ALARM) != 0U,
            now,
            BTN_ALARM_SHORT,
            BTN_ALARM_LONG);
        events |= update_button(
            &set,
            (input & BSP_BUTTON_SET) != 0U,
            now,
            BTN_SET_SHORT,
            BTN_SET_LONG);

        /* Start a separate hold interval when both debounced buttons are
         * down. Individual long events can occur at different times, so
         * combining their notification bits would not prove a joint hold. */
        if (!comboActive && alarm.stable && set.stable)
        {
            comboActive = 1U;
            comboHeld = 1U;
            comboSent = 0U;
            comboStartedAt = now;
        }

        if (comboActive)
        {
            if (!alarm.stable || !set.stable)
            {
                comboHeld = 0U;
            }
            if (comboHeld && !comboSent &&
                (TickType_t)(now - comboStartedAt) >= pdMS_TO_TICKS(BUTTON_LONG_MS))
            {
                comboSent = 1U;
                task_alarm_notify(BTN_RESET_DEFAULTS);
            }
            /* Swallow release events, including an aborted combo. Re-arm
             * only after BOTH buttons have finished release debouncing. */
            if (!alarm.stable && !set.stable && !alarm.raw && !set.raw)
            {
                comboActive = 0U;
            }
        }
        else if (events != 0UL)
        {
            task_alarm_notify(events);
        }
    }
}

BaseType_t task_button_create(void)
{
    return xTaskCreate(
        prvButtonTask,
        "BUTTON",
        192U,
        NULL,
        tskIDLE_PRIORITY + 3U,
        NULL);
}
