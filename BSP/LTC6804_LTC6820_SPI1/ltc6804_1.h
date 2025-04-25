#ifndef _LTC6804_1_H
#define _LTC6804_1_H

#include <stdio.h>
#include <stdlib.h>

#include "./BSP/LTC6804_LTC6820_SPI1/spi_1.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

// #define TOTAL_IC 2

/* LTC6804Ƭѡ���Ŷ��� */
#define CS_6804_GPIO_PORT GPIOB      /* PB6 */
#define CS_6804_GPIO_PIN  GPIO_PIN_6 /* 6804Ƭѡ�� */

/* LTC6804Ƭѡ���Ŷ˿ڲ��� */
#define CS_LTC6804(X)                                                             \
   do {                                                                           \
      X ? HAL_GPIO_WritePin(CS_6804_GPIO_PORT, CS_6804_GPIO_PIN, GPIO_PIN_SET)    \
        : HAL_GPIO_WritePin(CS_6804_GPIO_PORT, CS_6804_GPIO_PIN, GPIO_PIN_RESET); \
   } while (0)
/* LTC6804Ƭѡ���ŵ͵�ƽ��Ч����1ȡ��Ƭѡ��0ѡ�� */

void LTC6804_init(void);  // ltc6804��ʼ��

void LTC6804_adcv(void);  // ������ص�ѹת��

void LTC6804_adax(void);  // ����һ��ͨ�����������GPIO��ת��

void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG);
// ��ȫ�� ADC ���Ʊ���ӳ�䵽ÿ����ͬ ADC �����Ӧ���ʵ������ֽ�

uint8_t LTC6804_rdcv(uint8_t reg, uint8_t total_ic, uint16_t cell_codes[][12]);
// ��ȡ������ LTC6804 �ĵ�ص�ѹ�Ĵ�����

void LTC6804_rdcv_reg(uint8_t reg, uint8_t nIC, uint8_t* data);
// �� LTC6804 ��ص�ѹ�Ĵ�����ȡԭʼ����

int8_t LTC6804_rdaux(uint8_t reg, uint8_t nIC, uint16_t aux_codes[][6]);
// ��ȡ������ LTC6804 �����Ĵ���

void LTC6804_rdaux_reg(uint8_t reg, uint8_t nIC, uint8_t* data);
// �� LTC6804 �����Ĵ�����ȡԭʼ����

void LTC6804_clrcell(void);  // ��� LTC6804 ��ص�ѹ�Ĵ���

void LTC6804_clraux(void);  // ��� LTC6804 �����Ĵ���

void LTC6804_wrcfg(uint8_t nIC, uint8_t config[][6]);  // д�� LTC6804 ���üĴ���

int8_t LTC6804_rdcfg(uint8_t nIC, uint8_t r_config[][8]);  // ��ȡ LTC6804 ���������üĴ���

void wakeup_idle(void);  // ʹ isoSPI �ӿ���״̬����

void wakeup_sleep(void);  // ���Ѵ���˯��״̬�� LTC6804

uint16_t pec15_calc(uint8_t len, uint8_t* data);  // ���㲢���� CRC15 У��ֵ

void spi_write_array(uint8_t length, uint8_t* data);  // �Ӵ�������ӿڣ�SPI���˿����һ���ֽ����顣

void spi_write_read(uint8_t* TxData, uint8_t TXlen, uint8_t* rx_data, uint8_t RXlen);
// ʹ�ô�������ӿڣ�SPI���˿�д�벢��ȡһ���������ֽڡ�

//void init_cfg(void);

/**
 * MD����С��ADCת��ģʽ
 * 01��1������
 * 10��2������
 * 11��3���˲�
 */
#define MD_FAST        1
#define MD_NORMAL      2
#define MD_FILTERED    3

/**
 * CH(ͨ��ѡ��)����С��Ҫת����ͨ��
 * 000��0�����е�ص�Ԫ
 * 001��1����ص�Ԫ1��7
 * 010��2����ص�Ԫ2��8
 * 011��3����ص�Ԫ3��9
 * 100��4����ص�Ԫ4��10
 * 101��5����ص�Ԫ5��11
 * 110��6����ص�Ԫ6��12
 */
#define CELL_CH_ALL    0
#define CELL_CH_1and7  1
#define CELL_CH_2and8  2
#define CELL_CH_3and9  3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

/**
 * CHG(GPIOͨ��)����ֵ��Ҫת����ͨ��
 * 000��0������ͨ�����������GPIO�����ż��ڶ����ο���ѹ
 * 001��1��ͨ������������� 1
 * 010��2��ͨ������������� 2
 * 011��3��ͨ������������� 3
 * 100��4��ͨ������������� 4
 * 101��5��ͨ������������� 5
 * 110��6���ڶ����ο���ѹ��Vref2��
 */
#define AUX_CH_ALL     0
#define AUX_CH_GPIO1   1
#define AUX_CH_GPIO2   2
#define AUX_CH_GPIO3   3
#define AUX_CH_GPIO4   4
#define AUX_CH_GPIO5   5
#define AUX_CH_VREF2   6

/**
 * @brief   �����ڵ�ص�Ԫת���ڼ�ŵ羧��������û��ǽ��á�
 * DCP(�ŵ����)��ת���ڼ��Ƿ�����ŵ�
 * 0����-������ŵ�
 * 1����-����ŵ�
 */
#define DCP_DISABLED   0
#define DCP_ENABLED    1

/* CRC15��*/
static const unsigned int crc15Table[256] = {
    0x0,    0xc599, 0xceab, 0xb32,  0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac, 0xff35,
    0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a,
    0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e, 0x9eb7, 0x9585, 0x501c,
    0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4,
    0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544, 0x2be,  0xc727, 0xcc15, 0x98c,
    0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038,
    0x7c2,  0xc25b, 0xc969, 0xcf0,  0xdf0d, 0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560,
    0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8,
    0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de,
    0x8423, 0x41ba, 0x4a88, 0x8f11, 0x57c,  0xc0e5, 0xcbd7, 0xe4e,  0xddb3, 0x182a, 0x1318, 0xd681,
    0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
    0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9,
    0xf84,  0xca1d, 0xc12f, 0x4b6,  0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1,
    0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0,
    0xd3a,  0xc8a3, 0xc391, 0x608,  0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 0x5f98,
    0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350,
    0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3,
    0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc,
    0x846,  0xcddf, 0xc6ed, 0x374,  0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8,  0xcf61, 0xc453, 0x1ca,
    0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02,
    0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a,
    0x8ba7, 0x4e3e, 0x450c, 0x8095};

#endif  // _LTC6804_1_H
