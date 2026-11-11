#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

void HAL_GPIO_InitBoard(void);
void HAL_GPIO_BuzzerWrite(uint8_t on);
uint16_t HAL_GPIO_ButtonsPressedMask(void);
void HAL_GPIO_ButtonsIRQEnable(void);
uint32_t HAL_GPIO_ButtonIRQPending(void);
void HAL_GPIO_ClearButtonIRQ(uint32_t pending);

#endif
