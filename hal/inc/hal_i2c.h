#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>

void HAL_I2C_Init(void);
uint8_t HAL_I2C_Write(
    uint8_t address7,
    uint8_t control,
    const uint8_t *data,
    uint16_t length);

#endif
