#ifndef TFT_LCD_INTERNAL_H
#define TFT_LCD_INTERNAL_H
#include "lcd.h"
#include "lcd_port.h"
typedef struct {
    uint16_t x, y, w, h;
    uint16_t skip_x, skip_y;
    uint8_t clipped;
} LCD_Region;
extern uint8_t lcd_initialized;
LCD_Status lcd_command(uint8_t command, const uint8_t *data, size_t length);
LCD_Status lcd_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
LCD_Status lcd_clip(int32_t x, int32_t y, uint16_t w, uint16_t h, LCD_Region *region);
LCD_Status lcd_combine(LCD_Status a, LCD_Status b);
#endif
