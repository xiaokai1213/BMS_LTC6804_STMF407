#include "./BSP/LTC6804_LTC6820_SPI1/spi_1.h"

SPI_HandleTypeDef g_spi1_handler;  // SPI1句柄，用于SPI初始化

/**
 * @brief SPI1初始化
 * @note 主机模式，禁止硬件片选
 * @param 无
 * @retval 无
 */
void spi1_init(void) {
   __HAL_RCC_SPI1_CLK_ENABLE();                           // 使能SPI1的时钟
   g_spi1_handler.Instance = SPI1;                        // SPI1寄存器
   g_spi1_handler.Init.Mode = SPI_MODE_MASTER;            // 设置SPI工作模式，设置为主模式
   g_spi1_handler.Init.Direction = SPI_DIRECTION_2LINES;  // 设置SPI为双线模式
   g_spi1_handler.Init.DataSize = SPI_DATASIZE_8BIT;      // 设置SPI的数据大小:SPI发送接收8位帧结构
   g_spi1_handler.Init.CLKPolarity = SPI_POLARITY_HIGH;   // 串行同步时钟的空闲状态为高电平
   g_spi1_handler.Init.CLKPhase = SPI_PHASE_2EDGE;
   // 串行同步时钟的第二个跳变沿（上升或下降）数据被采样
   g_spi1_handler.Init.NSS = SPI_NSS_SOFT;
   // NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
   g_spi1_handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
   // 定义波特率预分频的值:波特率预分频值为256
   g_spi1_handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
   // 指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
   g_spi1_handler.Init.TIMode = SPI_TIMODE_DISABLE;                  // 关闭TI模式
   g_spi1_handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;  // 关闭硬件CRC校验
   g_spi1_handler.Init.CRCPolynomial = 7;                            // CRC值计算的多项式
   HAL_SPI_Init(&g_spi1_handler);                                    // 初始化SPI
   __HAL_SPI_ENABLE(&g_spi1_handler);                                // 使能SPI1
}

/**
 * @brief SPI1底层驱动，时钟使能，引脚配置，HAL_SPI_Init函数会调用本函数 
 * @param hspi:SPI句柄
 * @retval 无
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
   GPIO_InitTypeDef gpio_init_struct;  // GPIO口设置句柄
   if (hspi->Instance == SPI1) {       // 判断是否是SPI1的句柄
      __HAL_RCC_GPIOB_CLK_ENABLE();    // SPI1_SCK,SPI1_MISO,SPI1_MOSI三个引脚时钟使能

      /* SCK引脚模式设置(复用输出) */
      gpio_init_struct.Pin = SPI1_SCK_GPIO_PIN;              // SCK引脚（PB3）
      gpio_init_struct.Mode = GPIO_MODE_AF_PP;               // 复用推挽模式
      gpio_init_struct.Pull = GPIO_PULLUP;                   // 上拉电阻
      gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;         // 高速模式
      gpio_init_struct.Alternate = GPIO_AF5_SPI1;            // 复用至SPI1
      HAL_GPIO_Init(SPI1_SCK_GPIO_PORT, &gpio_init_struct);  // 设置SCK的GPIO

      /* MISO引脚模式设置(复用输出) */
      gpio_init_struct.Pin = SPI1_MISO_GPIO_PIN;              // MISO引脚（PB4）
      HAL_GPIO_Init(SPI1_MISO_GPIO_PORT, &gpio_init_struct);  // 设置MISO的GPIO

      /* MOSI引脚模式设置(复用输出) */
      gpio_init_struct.Pin = SPI1_MOSI_GPIO_PIN;              // MOSI引脚（PB5）
      HAL_GPIO_Init(SPI1_MOSI_GPIO_PORT, &gpio_init_struct);  // 设置MOSI的GPIO
   }
}

/**
 * @brief   SPI1速度设置函数
 * @note    SPI1时钟选择来自APB1，即PCLK1=42MHz
 *          SPI速度=PCLK1 / 2^(speed + 1)
 * @param   speed:SPI1时钟分频系数，取值为SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
 * @retval  无
 */
void spi1_set_speed(uint8_t speed) {
   assert_param(IS_SPI_BAUDRATE_PRESCALER(speed));  // 判断有效性
   __HAL_SPI_DISABLE(&g_spi1_handler);              // 关闭SPI
   g_spi1_handler.Instance->CR1 &= 0XFFC7;          // 位3-5清零，用来设置波特率
   g_spi1_handler.Instance->CR1 |= speed << 3;      // 设置SPI速度
   __HAL_SPI_ENABLE(&g_spi1_handler);               // 使能SPI
}

/**
 * @brief   SPI1读并写一个字节数据(8位数据)
 * @param   txbyte:要发送的数据（1个字节）
 * @retval  rxbyte:接收到的数据（1个字节）
 */
uint8_t spi1_r_w_byte(uint8_t txbyte) {
   uint8_t rxbyte;                                                       // 定义接收字节
   HAL_SPI_TransmitReceive(&g_spi1_handler, &txbyte, &rxbyte, 1, 1000);  // 调用收发函数
   return rxbyte;                                                        // 返回收到的数据
}
