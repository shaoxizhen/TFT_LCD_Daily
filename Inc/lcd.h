#ifndef TFT_LCD_H
#define TFT_LCD_H

#include <stddef.h>
#include <stdint.h>
#include "lcd_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LCD_OK = 0,
    LCD_CLIPPED,
    LCD_MISSING_GLYPH,
    LCD_INVALID_UTF8,
    LCD_INVALID_ARGUMENT,
    LCD_NOT_INITIALIZED,
    LCD_IO_ERROR,
    LCD_OVERFLOW
} LCD_Status;

typedef enum {
    LCD_FONT_ASCII24 = 0, /* 英文12x24；其他字符显示24x24缺字框。 */
    LCD_FONT_MIXED32     /* 中文32x32；英文12x24在32像素高的行中居中。 */
} LCD_Font;

#define LCD_COLOR_BLACK   UINT16_C(0x0000)
#define LCD_COLOR_WHITE   UINT16_C(0xFFFF)
#define LCD_COLOR_RED     UINT16_C(0xF800)
#define LCD_COLOR_GREEN   UINT16_C(0x07E0)
#define LCD_COLOR_BLUE    UINT16_C(0x001F)
#define LCD_COLOR_CYAN    UINT16_C(0x07FF)
#define LCD_COLOR_MAGENTA UINT16_C(0xF81F)
#define LCD_COLOR_YELLOW  UINT16_C(0xFFE0)
#define LCD_COLOR_GRAY    UINT16_C(0x8410)
#define LCD_RGB565(r,g,b) ((uint16_t)((((uint16_t)(r) & 0xF8U) << 8) | \
                                   (((uint16_t)(g) & 0xFCU) << 3) | \
                                   ((uint16_t)(b) >> 3)))

/* 阻塞接口，由主循环中的单一调用者使用。先完成CubeMX的GPIO/SPI初始化。 */
LCD_Status LCD_Init(void);
LCD_Status LCD_SetBacklight(uint8_t on);
const char *LCD_StatusName(LCD_Status status);
LCD_Status LCD_Clear(uint16_t color);
/* x,y是屏幕坐标；w,h是宽高，不是结束坐标。 */
LCD_Status LCD_FillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
LCD_Status LCD_DrawPoint(int16_t x, int16_t y, uint16_t color);
LCD_Status LCD_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
LCD_Status LCD_DrawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
/* 半径上限为屏幕长边的像素数。 */
LCD_Status LCD_DrawCircle(int16_t x, int16_t y, uint16_t radius, uint16_t color);
/* 图片按行排列，RGB565高字节在前；data_size填写数据字节数。 */
LCD_Status LCD_DrawImage(int16_t x, int16_t y, uint16_t w, uint16_t h,
                         const uint8_t *data, size_t data_size);

/* 以NUL结尾的UTF-8单行字符串；同时绘制文字和背景。 */
LCD_Status LCD_DrawText(int16_t x, int16_t y, const char *text, LCD_Font font,
                        uint16_t foreground, uint16_t background);
/* 初始化前也能测宽高。不自动换行；缺失字模按方框保留位置。 */
LCD_Status LCD_MeasureText(const char *text, LCD_Font font, uint16_t *width,
                           uint16_t *height);
/* 固定区域高24像素，右对齐并清除旧值。
 * 整个区域必须在屏幕内，否则返回CLIPPED且不绘制。 */
LCD_Status LCD_DrawInt(int16_t x, int16_t y, uint16_t field_width, int32_t value,
                       uint16_t foreground, uint16_t background);
/* decimals是小数位数，可选0..3。四舍五入，临界值受浮点二进制精度影响。
 * 数值上限随小数位变化：缩放并舍入后的绝对值需不超过UINT32_MAX。
 * NaN/无穷显示ERR；溢出显示OVER；区域放不下提示时仅清除并返回错误。 */
LCD_Status LCD_DrawNumber(int16_t x, int16_t y, uint16_t field_width, double value,
                          uint8_t decimals, uint16_t foreground, uint16_t background);

#ifdef __cplusplus
}
#endif
#endif
