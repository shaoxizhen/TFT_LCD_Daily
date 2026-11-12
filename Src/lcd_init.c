#include "lcd_internal.h"

uint8_t lcd_initialized;

typedef struct {
    uint8_t command, count, bytes[16];
} LCD_InitCommand;

/* 面板专用寄存器参数，继承用户提供的lcd_init.c。 */
static const LCD_InitCommand init_commands[] = {
    {0xB1,3,{0x05,0x3C,0x3C}},
    {0xB2,3,{0x05,0x3C,0x3C}},
    {0xB3,6,{0x05,0x3C,0x3C,0x05,0x3C,0x3C}},
    {0xB4,1,{0x03}},
    {0xC0,3,{0xAB,0x0B,0x04}},
    {0xC1,1,{0xC5}},
    {0xC2,2,{0x0D,0x00}},
    {0xC3,2,{0x8D,0x6A}},
    {0xC4,2,{0x8D,0xEE}},
    {0xC5,1,{0x0F}},
    {0xE0,16,{0x07,0x0E,0x08,0x07,0x10,0x07,0x02,0x07,0x09,0x0F,0x25,0x36,0x00,0x08,0x04,0x10}},
    {0xE1,16,{0x0A,0x0D,0x08,0x07,0x0F,0x07,0x02,0x07,0x09,0x0F,0x25,0x35,0x00,0x09,0x04,0x10}},
    {0xFC,1,{0x80}},
    {0x3A,1,{0x05}}
};

LCD_Status lcd_command(uint8_t command, const uint8_t *data, size_t length)
{
    int success;
    LCD_PortSelect(1);
    LCD_PortCommandMode(1);
    success = LCD_PortWrite(&command, 1);
    if (success && length != 0U) {
        LCD_PortCommandMode(0);
        success = LCD_PortWrite(data, length);
    }
    LCD_PortSelect(0);
    return success ? LCD_OK : LCD_IO_ERROR;
}

LCD_Status lcd_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t a, b;
    uint8_t bytes[4];
    LCD_Status status;
    a = (uint16_t)(x + LCD_X_OFFSET);
    b = (uint16_t)(a + w - 1U);
    bytes[0]=(uint8_t)(a>>8); bytes[1]=(uint8_t)a;
    bytes[2]=(uint8_t)(b>>8); bytes[3]=(uint8_t)b;
    status = lcd_command(0x2A, bytes, 4);
    if (status != LCD_OK) return status;
    a = (uint16_t)(y + LCD_Y_OFFSET);
    b = (uint16_t)(a + h - 1U);
    bytes[0]=(uint8_t)(a>>8); bytes[1]=(uint8_t)a;
    bytes[2]=(uint8_t)(b>>8); bytes[3]=(uint8_t)b;
    status = lcd_command(0x2B, bytes, 4);
    if (status != LCD_OK) return status;
    LCD_PortSelect(1);
    LCD_PortCommandMode(1);
    bytes[0]=0x2C;
    if (!LCD_PortWrite(bytes, 1)) {
        LCD_PortSelect(0);
        return LCD_IO_ERROR;
    }
    LCD_PortCommandMode(0);
    /* 调用者发送像素后释放GPIO片选，错误路径也必须释放。 */
    return LCD_OK;
}

LCD_Status LCD_Init(void)
{
    size_t i;
    LCD_Status status;
    static const uint8_t directions[4]={0x08,0xC8,0x78,0xA8};
    uint8_t direction=directions[LCD_ROTATION];
    lcd_initialized=0;
    LCD_PortBacklight(0);
    LCD_PortSelect(0);
    if (!LCD_PortReady()) return LCD_IO_ERROR;
    LCD_PortReset(1);
    LCD_PortDelay(10);
    LCD_PortReset(0);
    LCD_PortDelay(120);
    status=lcd_command(0x11, NULL, 0);
    if (status != LCD_OK) return status;
    LCD_PortDelay(120);
    for (i=0; i<sizeof(init_commands)/sizeof(init_commands[0]); ++i) {
        status=lcd_command(init_commands[i].command, init_commands[i].bytes,
                            init_commands[i].count);
        if (status != LCD_OK) return status;
    }
    status=lcd_command(0x36, &direction, 1);
    if (status != LCD_OK) return status;
    status=lcd_command(0x21, NULL, 0);
    if (status != LCD_OK) return status;
    status=lcd_command(0x29, NULL, 0);
    if (status != LCD_OK) return status;
    LCD_PortDelay(20);
    lcd_initialized=1;
    status=LCD_Clear(LCD_COLOR_BLACK);
    if (status != LCD_OK) { lcd_initialized=0; return status; }
    LCD_PortBacklight(1);
    return LCD_OK;
}

LCD_Status LCD_SetBacklight(uint8_t on)
{
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    LCD_PortBacklight(on != 0U);
    return LCD_OK;
}
