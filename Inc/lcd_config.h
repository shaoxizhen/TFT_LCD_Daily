#ifndef TFT_LCD_CONFIG_H
#define TFT_LCD_CONFIG_H

/* 按实际CubeMX工程修改：引脚还必须在CubeMX中同步配置。 */
#define LCD_HAL_HEADER "main.h"
#define LCD_SPI_HEADER "spi.h"
#define LCD_SPI_HANDLE hspi1
#define LCD_RES_PORT GPIOB
#define LCD_RES_PIN GPIO_PIN_0
#define LCD_DC_PORT GPIOB
#define LCD_DC_PIN GPIO_PIN_1
#define LCD_BLK_PORT GPIOB
#define LCD_BLK_PIN GPIO_PIN_10

/* 0：沿用PA4的SPI1硬件NSS；1：改用普通GPIO片选。 */
#ifndef LCD_CS_USE_GPIO
#define LCD_CS_USE_GPIO 0
#endif
#define LCD_CS_PORT GPIOA
#define LCD_CS_PIN GPIO_PIN_4

/* 继承原面板设置：0/1竖屏，2/3横屏，默认2。 */
#ifndef LCD_ROTATION
#define LCD_ROTATION 2
#endif
#if LCD_ROTATION == 0 || LCD_ROTATION == 1
#define LCD_WIDTH 80
#define LCD_HEIGHT 160
#ifndef LCD_X_OFFSET
#define LCD_X_OFFSET 26
#endif
#ifndef LCD_Y_OFFSET
#define LCD_Y_OFFSET 1
#endif
#elif LCD_ROTATION == 2 || LCD_ROTATION == 3
#define LCD_WIDTH 160
#define LCD_HEIGHT 80
#ifndef LCD_X_OFFSET
#define LCD_X_OFFSET 1
#endif
#ifndef LCD_Y_OFFSET
#define LCD_Y_OFFSET 26
#endif
#else
#error LCD_ROTATION_must_be_0_to_3
#endif

#define LCD_SPI_TIMEOUT_MS 100U
#define LCD_TRANSFER_BYTES 128U
#if LCD_TRANSFER_BYTES < 2 || (LCD_TRANSFER_BYTES % 2) != 0 || LCD_TRANSFER_BYTES > 65534
#error LCD_TRANSFER_BYTES_must_be_even_and_between_2_and_65534
#endif
#endif
