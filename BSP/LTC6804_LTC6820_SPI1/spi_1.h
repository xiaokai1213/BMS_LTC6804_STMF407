#ifndef _SPI_1_H
#define _SPI_1_H

#include "./SYSTEM/sys/sys.h"

/* SPI1���Ŷ��� */
#define SPI1_SCK_GPIO_PORT  GPIOB
#define SPI1_SCK_GPIO_PIN   GPIO_PIN_3 /* SPI1ʱ�ӿ� */

#define SPI1_MISO_GPIO_PORT GPIOB
#define SPI1_MISO_GPIO_PIN  GPIO_PIN_4 /* SPI1��������� */

#define SPI1_MOSI_GPIO_PORT GPIOB
#define SPI1_MOSI_GPIO_PIN  GPIO_PIN_5 /* SPI1��������� */

/* SPI�����ٶ����� */
#define SPI_SPEED_2         0
#define SPI_SPEED_4         1
#define SPI_SPEED_8         2
#define SPI_SPEED_16        3
#define SPI_SPEED_32        4
#define SPI_SPEED_64        5
#define SPI_SPEED_128       6
#define SPI_SPEED_256       7

/* �������� */
void spi1_init(void);                   // SPI1��ʼ��
void spi1_set_speed(uint8_t speed);     // SPI1�ٶ�����
uint8_t spi1_r_w_byte(uint8_t txbyte);  // SPI1��д

#endif  // _SPI_1_H
