#include "./BSP/LTC6804_LTC6820_SPI1/spi_1.h"

SPI_HandleTypeDef g_spi1_handler;  // SPI1���������SPI��ʼ��

/**
 * @brief SPI1��ʼ��
 * @note ����ģʽ����ֹӲ��Ƭѡ
 * @param ��
 * @retval ��
 */
void spi1_init(void) {
   __HAL_RCC_SPI1_CLK_ENABLE();                           // ʹ��SPI1��ʱ��
   g_spi1_handler.Instance = SPI1;                        // SPI1�Ĵ���
   g_spi1_handler.Init.Mode = SPI_MODE_MASTER;            // ����SPI����ģʽ������Ϊ��ģʽ
   g_spi1_handler.Init.Direction = SPI_DIRECTION_2LINES;  // ����SPIΪ˫��ģʽ
   g_spi1_handler.Init.DataSize = SPI_DATASIZE_8BIT;      // ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
   g_spi1_handler.Init.CLKPolarity = SPI_POLARITY_HIGH;   // ����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
   g_spi1_handler.Init.CLKPhase = SPI_PHASE_2EDGE;
   // ����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
   g_spi1_handler.Init.NSS = SPI_NSS_SOFT;
   // NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
   g_spi1_handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
   // ���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
   g_spi1_handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
   // ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
   g_spi1_handler.Init.TIMode = SPI_TIMODE_DISABLE;                  // �ر�TIģʽ
   g_spi1_handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;  // �ر�Ӳ��CRCУ��
   g_spi1_handler.Init.CRCPolynomial = 7;                            // CRCֵ����Ķ���ʽ
   HAL_SPI_Init(&g_spi1_handler);                                    // ��ʼ��SPI
   __HAL_SPI_ENABLE(&g_spi1_handler);                                // ʹ��SPI1
}

/**
 * @brief SPI1�ײ�������ʱ��ʹ�ܣ��������ã�HAL_SPI_Init��������ñ����� 
 * @param hspi:SPI���
 * @retval ��
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
   GPIO_InitTypeDef gpio_init_struct;  // GPIO�����þ��
   if (hspi->Instance == SPI1) {       // �ж��Ƿ���SPI1�ľ��
      __HAL_RCC_GPIOB_CLK_ENABLE();    // SPI1_SCK,SPI1_MISO,SPI1_MOSI��������ʱ��ʹ��

      /* SCK����ģʽ����(�������) */
      gpio_init_struct.Pin = SPI1_SCK_GPIO_PIN;              // SCK���ţ�PB3��
      gpio_init_struct.Mode = GPIO_MODE_AF_PP;               // ��������ģʽ
      gpio_init_struct.Pull = GPIO_PULLUP;                   // ��������
      gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;         // ����ģʽ
      gpio_init_struct.Alternate = GPIO_AF5_SPI1;            // ������SPI1
      HAL_GPIO_Init(SPI1_SCK_GPIO_PORT, &gpio_init_struct);  // ����SCK��GPIO

      /* MISO����ģʽ����(�������) */
      gpio_init_struct.Pin = SPI1_MISO_GPIO_PIN;              // MISO���ţ�PB4��
      HAL_GPIO_Init(SPI1_MISO_GPIO_PORT, &gpio_init_struct);  // ����MISO��GPIO

      /* MOSI����ģʽ����(�������) */
      gpio_init_struct.Pin = SPI1_MOSI_GPIO_PIN;              // MOSI���ţ�PB5��
      HAL_GPIO_Init(SPI1_MOSI_GPIO_PORT, &gpio_init_struct);  // ����MOSI��GPIO
   }
}

/**
 * @brief   SPI1�ٶ����ú���
 * @note    SPI1ʱ��ѡ������APB1����PCLK1=42MHz
 *          SPI�ٶ�=PCLK1 / 2^(speed + 1)
 * @param   speed:SPI1ʱ�ӷ�Ƶϵ����ȡֵΪSPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
 * @retval  ��
 */
void spi1_set_speed(uint8_t speed) {
   assert_param(IS_SPI_BAUDRATE_PRESCALER(speed));  // �ж���Ч��
   __HAL_SPI_DISABLE(&g_spi1_handler);              // �ر�SPI
   g_spi1_handler.Instance->CR1 &= 0XFFC7;          // λ3-5���㣬�������ò�����
   g_spi1_handler.Instance->CR1 |= speed << 3;      // ����SPI�ٶ�
   __HAL_SPI_ENABLE(&g_spi1_handler);               // ʹ��SPI
}

/**
 * @brief   SPI1����дһ���ֽ�����(8λ����)
 * @param   txbyte:Ҫ���͵����ݣ�1���ֽڣ�
 * @retval  rxbyte:���յ������ݣ�1���ֽڣ�
 */
uint8_t spi1_r_w_byte(uint8_t txbyte) {
   uint8_t rxbyte;                                                       // ��������ֽ�
   HAL_SPI_TransmitReceive(&g_spi1_handler, &txbyte, &rxbyte, 1, 1000);  // �����շ�����
   return rxbyte;                                                        // �����յ�������
}
