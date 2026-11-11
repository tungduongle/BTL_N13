#include "hal_i2c.h"

#include "bsp_pins.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_rcc.h"

#define I2C_TIMEOUT 150000UL

static uint8_t wait_event(uint32_t event)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (I2C_CheckEvent(BSP_I2C_PERIPH, event) == ERROR && --timeout > 0U)
    {
    }

    return (timeout > 0U) ? 1U : 0U;
}

void HAL_I2C_Init(void)
{
    I2C_InitTypeDef i2c;

    RCC_APB1PeriphClockCmd(BSP_I2C_CLOCK, ENABLE);
    I2C_DeInit(BSP_I2C_PERIPH);

    i2c.I2C_ClockSpeed = 100000U;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0U;
    i2c.I2C_Ack = I2C_Ack_Disable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init(BSP_I2C_PERIPH, &i2c);
    I2C_Cmd(BSP_I2C_PERIPH, ENABLE);
}

uint8_t HAL_I2C_Write(
    uint8_t address7,
    uint8_t control,
    const uint8_t *data,
    uint16_t length)
{
    uint32_t timeout = I2C_TIMEOUT;
    uint16_t index;

    while (I2C_GetFlagStatus(BSP_I2C_PERIPH, I2C_FLAG_BUSY) == SET && --timeout > 0U)
    {
    }
    if (timeout == 0U)
    {
        goto error;
    }

    I2C_GenerateSTART(BSP_I2C_PERIPH, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT) == 0U)
    {
        goto error;
    }

    I2C_Send7bitAddress(
        BSP_I2C_PERIPH,
        (uint8_t)(address7 << 1),
        I2C_Direction_Transmitter);
    if (wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == 0U)
    {
        goto error;
    }

    I2C_SendData(BSP_I2C_PERIPH, control);
    if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED) == 0U)
    {
        goto error;
    }

    for (index = 0U; index < length; ++index)
    {
        I2C_SendData(BSP_I2C_PERIPH, data[index]);
        if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED) == 0U)
        {
            goto error;
        }
    }

    I2C_GenerateSTOP(BSP_I2C_PERIPH, ENABLE);
    return 1U;

error:
    I2C_GenerateSTOP(BSP_I2C_PERIPH, ENABLE);
    I2C_SoftwareResetCmd(BSP_I2C_PERIPH, ENABLE);
    I2C_SoftwareResetCmd(BSP_I2C_PERIPH, DISABLE);
    HAL_I2C_Init();
    return 0U;
}
