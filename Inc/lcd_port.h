#ifndef TFT_LCD_PORT_H
#define TFT_LCD_PORT_H
#include <stddef.h>
#include <stdint.h>
/* 硬件传输适配；普通应用只需包含lcd.h。 */
int LCD_PortReady(void);
int LCD_PortWrite(const uint8_t *bytes, size_t length);
void LCD_PortCommandMode(uint8_t command_mode);
void LCD_PortSelect(uint8_t selected);
void LCD_PortReset(uint8_t asserted);
void LCD_PortBacklight(uint8_t on);
void LCD_PortDelay(uint32_t milliseconds);
#endif
