#include "ssd1306.h"
#include "bsp_board.h"
#include "app_config.h"

/* Minimal license-free self-contained 5x7 block glyphs. */
static const uint8_t digit[10][5] = {
 {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
 {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
 {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},
 {0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
 {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E}
};
static const uint8_t alpha[26][5] = {
 {0x7E,0x11,0x11,0x11,0x7E}, /* A */
 {0x7F,0x49,0x49,0x49,0x36}, /* B */
 {0x3E,0x41,0x41,0x41,0x22}, /* C */
 {0x7F,0x41,0x41,0x22,0x1C}, /* D */
 {0x7F,0x49,0x49,0x49,0x41}, /* E */
 {0x7F,0x09,0x09,0x09,0x01}, /* F */
 {0x3E,0x41,0x49,0x49,0x7A}, /* G */
 {0x7F,0x08,0x08,0x08,0x7F}, /* H */
 {0x00,0x41,0x7F,0x41,0x00}, /* I */
 {0x20,0x40,0x41,0x3F,0x01}, /* J */
 {0x7F,0x08,0x14,0x22,0x41}, /* K */
 {0x7F,0x40,0x40,0x40,0x40}, /* L */
 {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
 {0x7F,0x04,0x08,0x10,0x7F}, /* N */
 {0x3E,0x41,0x41,0x41,0x3E}, /* O */
 {0x7F,0x09,0x09,0x09,0x06}, /* P */
 {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
 {0x7F,0x09,0x19,0x29,0x46}, /* R */
 {0x46,0x49,0x49,0x49,0x31}, /* S */
 {0x01,0x01,0x7F,0x01,0x01}, /* T */
 {0x3F,0x40,0x40,0x40,0x3F}, /* U */
 {0x1F,0x20,0x40,0x20,0x1F}, /* V */
 {0x3F,0x40,0x38,0x40,0x3F}, /* W */
 {0x63,0x14,0x08,0x14,0x63}, /* X */
 {0x07,0x08,0x70,0x08,0x07}, /* Y */
 {0x61,0x51,0x49,0x45,0x43}  /* Z */
};
static uint8_t frame[SSD1306_WIDTH * SSD1306_HEIGHT / 8U];

static uint8_t send_cmd(uint8_t command)
{
    return bsp_i2c1_write(SSD1306_I2C_ADDR, 0x00U, &command, 1U);
}

uint8_t SSD1306_Init(void)
{
    static const uint8_t initSeq[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0x7F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    uint16_t index;

    for (index = 0U; index < sizeof(initSeq); ++index)
    {
        if (!send_cmd(initSeq[index]))
        {
            return 0U;
        }
    }

    SSD1306_Clear();
    return SSD1306_UpdateScreen();
}

void SSD1306_Clear(void)
{
    uint16_t index;

    for (index = 0U; index < sizeof(frame); ++index)
    {
        frame[index] = 0U;
    }
}

void SSD1306_Pixel(uint8_t x, uint8_t y, uint8_t on)
{
    uint16_t index;
    uint8_t mask;

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        return;
    }

    index = (uint16_t)x + ((uint16_t)y / 8U) * SSD1306_WIDTH;
    mask = (uint8_t)(1U << (y & 7U));
    if (on)
    {
        frame[index] |= mask;
    }
    else
    {
        frame[index] &= (uint8_t)~mask;
    }
}

void SSD1306_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int x = (int)x0;
    int y = (int)y0;
    int dx = ((int)x1 > (int)x0) ? ((int)x1 - (int)x0) : ((int)x0 - (int)x1);
    int dy = ((int)y1 > (int)y0) ? ((int)y0 - (int)y1) : ((int)y1 - (int)y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    int e2;

    for (;;)
    {
        SSD1306_Pixel((uint8_t)x, (uint8_t)y, 1U);
        if (x == (int)x1 && y == (int)y1)
        {
            break;
        }

        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y += sy;
        }
    }
}

static const uint8_t *glyph(char character)
{
    static const uint8_t space[5] = {0, 0, 0, 0, 0};
    static const uint8_t colon[5] = {0, 0x36, 0x36, 0, 0};
    static const uint8_t dash[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
    static const uint8_t dot[5] = {0, 0x60, 0x60, 0, 0};
    static const uint8_t arrow[5] = {0x08, 0x14, 0x22, 0x41, 0};
    static const uint8_t slash[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t question[5] = {0x02, 0x01, 0x51, 0x09, 0x06};

    if (character >= '0' && character <= '9')
    {
        return digit[character - '0'];
    }
    if (character >= 'A' && character <= 'Z')
    {
        return alpha[character - 'A'];
    }
    if (character >= 'a' && character <= 'z')
    {
        return alpha[character - 'a'];
    }
    if (character == ':')
    {
        return colon;
    }
    if (character == '-')
    {
        return dash;
    }
    if (character == '.')
    {
        return dot;
    }
    if (character == '>')
    {
        return arrow;
    }
    if (character == '/')
    {
        return slash;
    }
    if (character == '?')
    {
        return question;
    }

    return space;
}

void SSD1306_String(uint8_t x, uint8_t y, const char *text)
{
    uint8_t column;
    uint8_t row;
    const uint8_t *bits;

    while (*text != '\0' && x <= 122U)
    {
        bits = glyph(*text++);
        for (column = 0U; column < 5U; ++column)
        {
            for (row = 0U; row < 7U; ++row)
            {
                if (bits[column] & (1U << row))
                {
                    SSD1306_Pixel(x + column, y + row, 1U);
                }
            }
        }
        x = (uint8_t)(x + 6U);
    }
}

void SSD1306_Number(uint8_t x, uint8_t y, uint16_t value)
{
    char str[6];
    uint8_t count = 0U;
    uint8_t index;

    do
    {
        str[count++] = (char)('0' + value % 10U);
        value = (uint16_t)(value / 10U);
    } while (value && count < 5U);

    for (index = 0U; index < count; ++index)
    {
        char digitChar[2];

        digitChar[0] = str[count - 1U - index];
        digitChar[1] = '\0';
        SSD1306_String((uint8_t)(x + 6U * index), y, digitChar);
    }
}

uint8_t SSD1306_UpdateScreen(void)
{
    uint8_t page;
    uint8_t commands[3];

    for (page = 0U; page < 8U; ++page)
    {
        commands[0] = (uint8_t)(0xB0U | page);
        commands[1] = 0x00U;
        commands[2] = 0x10U;

        if (!bsp_i2c1_write(SSD1306_I2C_ADDR, 0x00U, commands, 3U))
        {
            return 0U;
        }
        if (!bsp_i2c1_write(
                SSD1306_I2C_ADDR,
                0x40U,
                &frame[(uint16_t)page * SSD1306_WIDTH],
                SSD1306_WIDTH))
        {
            return 0U;
        }
    }

    return 1U;
}
