#ifndef TFT_LCD_FONT_H
#define TFT_LCD_FONT_H
#include <stdint.h>
typedef struct {
    uint32_t codepoint;
    uint8_t pixels[128]; /* 32行，每行4字节，低位在前。 */
} LCD_ChineseGlyph;
extern const uint8_t lcd_ascii_12x24[95][48];
extern const LCD_ChineseGlyph lcd_chinese_32[];
extern const uint16_t lcd_chinese_count;
#endif
