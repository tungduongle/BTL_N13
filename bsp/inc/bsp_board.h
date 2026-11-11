#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#include <stdint.h>

#define BSP_ADC_INVALID 0xFFFFU
#define BSP_BUTTON_ALARM 0x1000U
#define BSP_BUTTON_SET   0x2000U

void bsp_board_init(void);
void bsp_buttons_irq_enable(void);
uint16_t bsp_buttons_pressed_mask(void);
uint16_t bsp_adc_read(void);
void bsp_buzzer_write(uint8_t on);
uint8_t bsp_i2c1_write(
    uint8_t addr7,
    uint8_t control,
    const uint8_t *bytes,
    uint16_t len);
uint8_t bsp_flash_load_thresholds(uint16_t *warning, uint16_t *danger);
uint8_t bsp_flash_save_thresholds(uint16_t warning, uint16_t danger);

#endif
