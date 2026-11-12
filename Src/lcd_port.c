#include "lcd_config.h"
#include LCD_HAL_HEADER
#include LCD_SPI_HEADER
#include "lcd_port.h"

int LCD_PortReady(void)
{
    return HAL_SPI_GetState(&LCD_SPI_HANDLE) == HAL_SPI_STATE_READY;
}

int LCD_PortWrite(const uint8_t *bytes, size_t length)
{
    while (length != 0U) {
        uint16_t count = (uint16_t)(length > LCD_TRANSFER_BYTES ? LCD_TRANSFER_BYTES : length);
        if (HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)bytes, count,
                             LCD_SPI_TIMEOUT_MS) != HAL_OK) return 0;
        bytes += count;
        length -= count;
    }
    return 1;
}

void LCD_PortCommandMode(uint8_t command_mode)
{
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN,
                      command_mode ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void LCD_PortSelect(uint8_t selected)
{
#if LCD_CS_USE_GPIO
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN,
                      selected ? GPIO_PIN_RESET : GPIO_PIN_SET);
#else
    /* 硬件NSS由SPI的SPE控制，本驱动不在发送间隙关闭SPI。 */
    (void)selected;
#endif
}

void LCD_PortReset(uint8_t asserted)
{
    HAL_GPIO_WritePin(LCD_RES_PORT, LCD_RES_PIN,
                      asserted ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void LCD_PortBacklight(uint8_t on)
{
    HAL_GPIO_WritePin(LCD_BLK_PORT, LCD_BLK_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LCD_PortDelay(uint32_t milliseconds) { HAL_Delay(milliseconds); }
