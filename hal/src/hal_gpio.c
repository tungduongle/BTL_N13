#include "hal_gpio.h"

#include "bsp_pins.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void HAL_GPIO_InitBoard(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,
        ENABLE);

    gpio.GPIO_Pin = BSP_SENSOR_ADC_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(BSP_SENSOR_ADC_PORT, &gpio);

    gpio.GPIO_Pin = BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(BSP_I2C_PORT, &gpio);

    gpio.GPIO_Pin = BSP_BUZZER_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BSP_BUZZER_PORT, &gpio);
    HAL_GPIO_BuzzerWrite(0U);

    gpio.GPIO_Pin = BSP_BUTTON_ALARM_PIN | BSP_BUTTON_SET_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &gpio);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);

    exti.EXTI_Line = BSP_BUTTON_ALARM | BSP_BUTTON_SET;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);
    EXTI_ClearITPendingBit(exti.EXTI_Line);
}

void HAL_GPIO_BuzzerWrite(uint8_t on)
{
    if (on != 0U)
    {
        GPIO_ResetBits(BSP_BUZZER_PORT, BSP_BUZZER_PIN);
    }
    else
    {
        GPIO_SetBits(BSP_BUZZER_PORT, BSP_BUZZER_PIN);
    }
}

uint16_t HAL_GPIO_ButtonsPressedMask(void)
{
    uint16_t pressed = 0U;

    if (GPIO_ReadInputDataBit(BSP_BUTTON_ALARM_PORT, BSP_BUTTON_ALARM_PIN) == Bit_RESET)
    {
        pressed |= BSP_BUTTON_ALARM;
    }
    if (GPIO_ReadInputDataBit(BSP_BUTTON_SET_PORT, BSP_BUTTON_SET_PIN) == Bit_RESET)
    {
        pressed |= BSP_BUTTON_SET;
    }

    return pressed;
}

void HAL_GPIO_ButtonsIRQEnable(void)
{
    NVIC_InitTypeDef nvic;

    EXTI_ClearITPendingBit(BSP_BUTTON_ALARM | BSP_BUTTON_SET);

    nvic.NVIC_IRQChannel = BSP_BUTTON_EXTI_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

uint32_t HAL_GPIO_ButtonIRQPending(void)
{
    uint32_t pending = 0U;

    if (EXTI_GetITStatus(BSP_BUTTON_ALARM) == SET)
    {
        pending |= BSP_BUTTON_ALARM;
    }
    if (EXTI_GetITStatus(BSP_BUTTON_SET) == SET)
    {
        pending |= BSP_BUTTON_SET;
    }

    return pending;
}

void HAL_GPIO_ClearButtonIRQ(uint32_t pending)
{
    EXTI_ClearITPendingBit(pending);
}
