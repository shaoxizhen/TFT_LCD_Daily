# TFT_LCD_Daily

面向 STM32 HAL 的 ST7735S 0.96 英寸（80×160）SPI 彩屏驱动。当前版本 v1.0.0，默认横屏 160×80。

支持 RGB565 绘图、图片、UTF-8 文字和固定区域数字显示。采用阻塞传输，无全屏缓冲；在主循环中顺序调用。

## 文件怎么用

- `Src/`：4 个 `.c` 文件，全部加入工程编译。
- `Inc/`：5 个头文件，保留整个目录，并将该目录添加到 Include Paths。
- `Inc/lcd_config.h`：SPI 句柄、引脚、屏幕方向和偏移配置。
- `Src/lcd_port.c`：HAL 适配及背光控制。
- [先看这里.html](./先看这里.html)：完整中文使用说明，下载后用浏览器打开。

工程自行提供 `main.h`、`spi.h` 及 HAL、时钟、GPIO 和 SPI 初始化。旧版同名驱动应取消参与编译，避免重复定义。

## 接线必须与配置一致

仓库保留以下默认配置；它不是对任意接线自动适配的驱动：

| 屏幕信号 | 默认配置 |
| --- | --- |
| GND / VCC | 共地 / 3.3V |
| SCL / SDA | PA5 / PA7（SPI1 SCK / MOSI） |
| RES / DC | PB0 / PB1（推挽输出） |
| CS | PA4（SPI1 硬件 NSS 输出） |
| BLK | PB10（推挽输出，高电平点亮） |

SPI 使用主机、8 位、MSB first、CPOL 低、CPHA 第 1 边沿。修改 `lcd_config.h` 后，还需同步工程的 GPIO/SPI 配置。

如果使用普通 GPIO 片选，将 `LCD_CS_USE_GPIO` 设为 `1`，并修改 `LCD_CS_PORT`、`LCD_CS_PIN`；SPI 配置为软件 NSS。

**例如实物 CS=PB0、RES=PB10 时，不能直接使用默认配置。** 应改为 GPIO 片选 PB0、复位 PB10。若 BLK 直接接 3.3V，还应将 `LCD_PortBacklight()` 改为空操作，避免继续把 PB10 当作背光输出；此时背光常亮，无法通过 `LCD_SetBacklight()` 开关。RES 和 BLK 不能共用一个 GPIO。

## main.c 最小用法

```c
#include "lcd.h"

/* 放在 HAL、时钟、MX_GPIO_Init() 和 MX_SPI1_Init() 完成之后 */
LCD_Status status = LCD_Init();
if (status != LCD_OK) {
    Error_Handler();
}

status = LCD_Clear(LCD_COLOR_WHITE);
if (status != LCD_OK) {
    Error_Handler();
}
status = LCD_DrawText(4, 4, "Hello", LCD_FONT_ASCII24,
                      LCD_COLOR_BLUE, LCD_COLOR_WHITE);
if (status != LCD_OK) {
    Error_Handler();
}
```

更新数值时调用 `LCD_DrawInt()` 或 `LCD_DrawNumber()`：数字区域高 24 像素，宽度由调用者指定，每次清除旧值并右对齐。矩形参数为 x、y、宽、高。

内置 ASCII 字体为 12×24；中文字模仅有“电、流、压”（32×32）。源文件字符串使用 UTF-8，其他汉字需要补充字模。

## 适用范围

初始化参数及字模沿用所提供的原始驱动，面板方向和偏移需按实际屏幕确认。这里提供驱动源码，不是完整的 STM32 工程。

`LCD_OK` 表示软件操作完成，不能证明屏幕接线或硬件正常。完整接口、返回状态和使用限制见中文 HTML 说明。本次上传保留现有驱动内容，未进行新的上板验证。
