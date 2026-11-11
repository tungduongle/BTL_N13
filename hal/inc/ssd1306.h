#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_WIDTH  128U
#define SSD1306_HEIGHT 64U

uint8_t SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_Pixel(uint8_t x, uint8_t y, uint8_t on);
void SSD1306_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void SSD1306_String(uint8_t x, uint8_t y, const char *text);
void SSD1306_Number(uint8_t x, uint8_t y, uint16_t value);
uint8_t SSD1306_UpdateScreen(void);

#endif
