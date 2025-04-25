#ifndef _SPI_1_H
#define _SPI_1_H

#include "./SYSTEM/sys/sys.h"

/* SPI1引脚定义 */
#define SPI1_SCK_GPIO_PORT  GPIOB
#define SPI1_SCK_GPIO_PIN   GPIO_PIN_3 /* SPI1时钟口 */

#define SPI1_MISO_GPIO_PORT GPIOB
#define SPI1_MISO_GPIO_PIN  GPIO_PIN_4 /* SPI1主机输入口 */

#define SPI1_MOSI_GPIO_PORT GPIOB
#define SPI1_MOSI_GPIO_PIN  GPIO_PIN_5 /* SPI1主机输出口 */

/* SPI总线速度设置 */
#define SPI_SPEED_2         0
#define SPI_SPEED_4         1
#define SPI_SPEED_8         2
#define SPI_SPEED_16        3
#define SPI_SPEED_32        4
#define SPI_SPEED_64        5
#define SPI_SPEED_128       6
#define SPI_SPEED_256       7

/* 函数声明 */
void spi1_init(void);                   // SPI1初始化
void spi1_set_speed(uint8_t speed);     // SPI1速度设置
uint8_t spi1_r_w_byte(uint8_t txbyte);  // SPI1读写

#endif  // _SPI_1_H
