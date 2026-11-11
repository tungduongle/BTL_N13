#ifndef BSP_PINS_H
#define BSP_PINS_H

#include "stm32f10x.h"
#include "bsp_board.h"

#define BSP_BUZZER_PORT          GPIOB
#define BSP_BUZZER_PIN           GPIO_Pin_11
#define BSP_BUTTON_ALARM_PORT    GPIOB
#define BSP_BUTTON_ALARM_PIN     GPIO_Pin_12
#define BSP_BUTTON_SET_PORT      GPIOB
#define BSP_BUTTON_SET_PIN       GPIO_Pin_13
#define BSP_SENSOR_ADC_PORT      GPIOA
#define BSP_SENSOR_ADC_PIN       GPIO_Pin_0
#define BSP_SENSOR_ADC_CHANNEL   ADC_Channel_0
#define BSP_I2C_PORT             GPIOB
#define BSP_I2C_SCL_PIN          GPIO_Pin_6
#define BSP_I2C_SDA_PIN          GPIO_Pin_7
#define BSP_I2C_PERIPH           I2C1
#define BSP_I2C_CLOCK            RCC_APB1Periph_I2C1
#define BSP_BUTTON_EXTI_IRQn     EXTI15_10_IRQn

#endif
