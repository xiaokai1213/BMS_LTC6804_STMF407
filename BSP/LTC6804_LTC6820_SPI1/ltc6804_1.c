#include "./BSP/LTC6804_LTC6820_SPI1/ltc6804_1.h"
/*
uint8_t tx_cfg[6][6];
uint8_t rx_cfg[6][8];
static uint8_t TOTAL_IC = 1;

void init_cfg(void) {
   int i;
   for (i = 0; i < TOTAL_IC; i++) {
      tx_cfg[i][0] = 0xFE;  // GPIO引脚下拉电路关断（bit8~bit4） | 基准保持上电状态（bit3） |
                            // SWTEN处于逻辑1（软件定时器） | ADC模式选择为0
      tx_cfg[i][1] = 0x00;  // 不使用欠压比较功能
      tx_cfg[i][2] = 0x00;  // 不使用过压比较功能
      tx_cfg[i][3] = 0x00;
      tx_cfg[i][4] = 0x00;  // 不使用电池放电功能
      tx_cfg[i][5] = 0xF0;  // 软件放电超时时间120min
   }
}
*/
/* 6804 转换命令变量 */
uint8_t ADCV[2];  // 电池电压转换命令
uint8_t ADAX[2];  // 通用输入输出（GPIO）转换命令

/**
 * @brief   初始化LTC6804的片选IO,并初始化SPI1外设
 * @note    自检6804程序
 * @param   无
 * @retval  无
 */
void LTC6804_init(void) {
   __HAL_RCC_GPIOB_CLK_ENABLE();                         // 6804片选脚时钟使能
   GPIO_InitTypeDef gpio_init_struct;                    // 定义gpio初始化结构体
   gpio_init_struct.Pin = CS_6804_GPIO_PIN;              // cs6804引脚（PB6）
   gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;          // 推挽输出
   gpio_init_struct.Pull = GPIO_PULLUP;                  // 上拉电阻
   gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        // 引脚高速模式
   HAL_GPIO_Init(CS_6804_GPIO_PORT, &gpio_init_struct);  // 设置CS的gpio
   CS_LTC6804(1);                                        // 取消片选

   spi1_init();  // 初始化SPI1
   // spi1_set_speed(SPI_SPEED_128);  // 设置SPI1的速度

   // init_cfg();                                                 // 配置LTC6804的寄存器
   uint8_t tx_cfg[6][6];
   uint8_t rx_cfg[6][8];
   uint8_t TOTAL_IC = 1;
   int i;
   for (i = 0; i < TOTAL_IC; i++) {
      tx_cfg[i][0] = 0xFE;  // GPIO引脚下拉电路关断（bit8~bit4） | 基准保持上电状态（bit3） |
      tx_cfg[i][1] = 0x00;  // SWTEN处于逻辑1（软件定时器） | ADC模式选择为0
      tx_cfg[i][2] = 0x00;  // 不使用欠压比较功能
      tx_cfg[i][3] = 0x00;  // 不使用过压比较功能
      tx_cfg[i][4] = 0x00;  // 不使用电池放电功能
      tx_cfg[i][5] = 0xF0;  // 软件放电超时时间120min
   }
   set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL);  // 设置adc转化命令
   wakeup_sleep();                                             // 唤醒芯片
   LTC6804_adcv();                                             //
   LTC6804_wrcfg(TOTAL_IC, tx_cfg);                            // 把上面的设置写入芯片
   wakeup_idle();
   wakeup_idle();                               // 防止数据第一个byte变为255，REFON=1
   if (LTC6804_rdcfg(TOTAL_IC, rx_cfg) == 1) {  // 检查一下到底有没有配置成功
      printf("LTC6804_MODULAR INIT NG!\n\r");
   } else {
      printf("LTC6804_MODULAR INIT OK!\n\r");
   }
}

/**
 * @brief  将全局 ADC 控制变量映射到每个不同 ADC 命令对应的适当控制字节。
 * @param   MD: ADC 转换模式
 * @param   DCP：控制在电池单元转换期间是否允许放电
 * @param   CH：确定在 ADC 转换命令执行期间测量哪些电池单元
 * @param   CHG：确定在辅助转换命令执行期间测量哪些通用输入输出（GPIO）通道
 * @retval  无
 */
void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG) {
   uint8_t md_bits;

   md_bits = (MD & 0x02) >> 1;
   ADCV[0] = md_bits + 0x02;
   md_bits = (MD & 0x01) << 7;
   ADCV[1] = md_bits + 0x60 + (DCP << 4) + CH;

   md_bits = (MD & 0x02) >> 1;
   ADAX[0] = md_bits + 0x04;
   md_bits = (MD & 0x01) << 7;
   ADAX[1] = md_bits + 0x60 + CHG;
}
/*
|command	|  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |
|-----------|-------|-------|-------|-------|-------|-------|-------|-------|
|ADCV:	    |   0   |   0   |   0   |   0   |   0   |   0   |   1   | MD[1] |
|ADAX:	    |   0   |   0   |   0   |   0   |   0   |   1   |   0   | MD[1] |
*****************************************************************************
|command	|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
|-----------|-------|-------|-------|-------|-------|-------|-------|-------|
|ADCV:	    | MD[2] |   1   |   1   |  DCP  |   0   | CH[2] | CH[1] | CH[0] |
|ADAX:	    | MD[2] |   1   |   1   |  DCP  |   0   | CHG[2]| CHG[1]| CHG[0]|
*/

/**
 * @brief   启动电池电压转换。
 *          启动 LTC6804 的 C 引脚输入的模数（ADC）转换。
 *          可以通过设置相关的全局变量来更改所执行的 ADC 转换类型。
 * @param   无
 * @retval  无
 */
void LTC6804_adcv(void) {
   uint8_t cmd[4];
   uint16_t cmd_pec;

   // 1
   cmd[0] = ADCV[0];
   cmd[1] = ADCV[1];

   // 2
   cmd_pec = pec15_calc(2, ADCV);
   cmd[2] = (uint8_t)(cmd_pec >> 8);
   cmd[3] = (uint8_t)(cmd_pec);

   // 3
   wakeup_idle();
   // 这将确保 LTC6804 的隔离式串行外设接口（isoSPI）端口处于唤醒状态。此命令可以删除。

   // 4
   CS_LTC6804(0);            // 拉低片选
   spi_write_array(4, cmd);  //
   CS_LTC6804(1);            // 拉高取消片选
}
/*
LTC6804_adcv 函数序列：
将 adcv 命令加载到命令数组中。
计算 adcv 命令的奇偶校验码（PEC），并将其加载到命令数组中。
唤醒隔离 SPI（isoSPI）端口。如果此前已确保 isoSPI 的状态，则此步骤可以省略。
向 LTC6804 菊链发送广播 adcv 命令。
*/

/**
 * @brief   启动一次通用输入输出（GPIO）转换
 *          启动 LTC6804 的 GPIO 输入的模数（ADC）转换。
 *          可通过设置相关的全局变量来更改所执行的 ADC 转换类型
 * @param   无
 * @retval  无
 */
void LTC6804_adax(void) {
   uint8_t cmd[4];
   uint16_t cmd_pec;

   cmd[0] = ADAX[0];
   cmd[1] = ADAX[1];
   cmd_pec = pec15_calc(2, ADAX);
   cmd[2] = (uint8_t)(cmd_pec >> 8);
   cmd[3] = (uint8_t)(cmd_pec);

   wakeup_idle();
   // 这将确保 LTC6804 的隔离式串行外设接口（isoSPI）端口处于唤醒状态。此命令是可以去掉的。

   CS_LTC6804(0);            // 拉低片选
   spi_write_array(4, cmd);  //
   CS_LTC6804(1);            // 拉高取消片选
}
/*
LTC6804_adax 函数执行步骤：
将 adax 命令载入命令数组。
计算 adax 命令的循环冗余校验码（PEC），并将其载入命令数组。
唤醒隔离式串行外设接口（isoSPI）端口。若此前已确保 isoSPI 端口状态正常，此步骤可省略。
向 LTC6804 菊链发送广播 adax 命令。
*/

/**
 * @brief   读取并解析 LTC6804 的电池电压寄存器。
 *          此函数用于读取 LTC6804 的电池编码。
 *          该函数会发送所需的读取命令，解析数据，并将电池电压存储在cell_codes变量中。
 * @param   reg:此参数用于控制读取哪个电池电压寄存器。
 *              0：读取所有电池寄存器。
 *              1：读取电池组 A。
 *              2：读取电池组 B。
 *              3：读取电池组 C。
 *              4：读取电池组 D。
 * @param   total_ic:这是菊链中集成电路（IC）的数量（仅取 -1）。
 * @param   cell_codes[][12]:这是一个存储解析后的电池编码的数组，按从低到高的顺序排列。
 *              电池编码将按以下格式存储在 cell_codes[] 数组中：
 *              cell_codes[i][j]:i表示IC数量，j表示单个IC的电池由0-12
 * @retval  奇偶校验码（PEC）状态。
 *          0：未检测到 PEC 错误。
 *          1：检测到 PEC 错误，需重试读取。
 */
uint8_t LTC6804_rdcv(uint8_t reg, uint8_t total_ic, uint16_t cell_codes[][12]) {
   const uint8_t NUM_RX_BYT = 8;
   const uint8_t BYT_IN_REG = 6;
   const uint8_t CELL_IN_REG = 3;

   uint8_t* cell_data;
   uint8_t pec_error = 0;
   uint16_t parsed_cell;
   uint16_t received_pec;
   uint16_t data_pec;
   uint8_t data_counter = 0;  // 数据计数器
   cell_data = (uint8_t*)malloc((NUM_RX_BYT * total_ic) * sizeof(uint8_t));
   // 1.a
   if (reg == 0) {
      // a.i
      for (uint8_t cell_reg = 1; cell_reg < 5; cell_reg++) {
         // 对于 LTC6804 的每个电池电压寄存器都执行一次。
         data_counter = 0;
         LTC6804_rdcv_reg(cell_reg, total_ic, cell_data);  // 读取单个电池电压寄存器

         for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
            // 对菊链中的每一个LTC6804都执行（该操作）。当前的
            // “current_ic”被用作集成电路（IC）计数器。
            // a.ii
            for (uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++) {
               // 这个循环把读回的数据解析成电池电压，针对寄存器里的 3 个电池电压编码各执行一次。
               parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
               // 每个电池编码以两个字节的形式接收，然后将这两个字节组合起来，以生成解析后的电池电压编码。

               cell_codes[current_ic][current_cell + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
               data_counter = data_counter + 2;
               // 由于电池电压编码是两个字节，因此对于每个已解析的电池编码，数据计数器必须递增 2。
            }
            // a.iii
            received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter + 1];
            // 当前集成电路（IC）接收到的循环冗余校验码（PEC）会在 6 个电池电压数据字节之后，以第 7
            // 个和第 8 个字节的形式进行传输。
            data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
            if (received_pec != data_pec) {
               pec_error = 1;
               // 如果在串行数据中检测到任何奇偶校验错误（PEC 错误），`pec_error`
               // 变量就会被简单地设置为负值。
            }
            data_counter = data_counter + 2;
            // 由于传输的奇偶校验码（PEC）为 2 字节长，因此数据计数器必须递增 2
            // 字节，以指向接下来的集成电路（IC）的电池电压数据。
         }
      }
   }
   // 1.b
   else {
      // b.i
      LTC6804_rdcv_reg(reg, total_ic, cell_data);
      for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
         // 对于菊链中的每个 LTC6804 都会执行（相应操作），`current_ic`用作集成电路（IC）计数器。
         // b.ii
         for (uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++) {
            // 这个循环将读回的数据解析为电池电压，对于寄存器中的三个电池电压编码中的每一个，它都会循环一次。

            parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
            // 每个电池编码以两个字节的形式接收，然后将这两个字节合并，从而生成解析后的电池电压编码。

            cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)] =
                0x0000FFFF & parsed_cell;
            data_counter = data_counter + 2;
            // 因为电池电压编码是两个字节，所以对于每一个已解析的电池编码，数据计数器都必须增加 2 。
         }
         // b.iii
         received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter + 1];
         // 当前集成电路（current_ic）所接收到的奇偶校验码（PEC）会在 6
         // 个电池电压数据字节之后，作为第 7 和第 8 个字节进行传输。
         data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
         if (received_pec != data_pec) {
            pec_error = 1;
            // 如果在串行数据中检测到任何奇偶校验码（PEC）错误，那么“pec_error”变量就会简单地被设置为负值。
         }
         data_counter = data_counter + 2;
         // 由于传输的奇偶校验码（PEC）长度为2个字节，因此数据计数器必须递增2个字节，以便指向接下来的集成电路（IC）的电池电压数据。
      }
   }
   // 2
   free(cell_data);
   return (pec_error);
}
/*
LTC6804_rdcv 序列
1. 开关语句：
a. 寄存器值（Reg）等于 0：
i. 读取菊链中每个集成电路（IC）的电池电压寄存器 A - D：对菊链里的每一个 IC，获取其电池电压寄存器 A
到 D 的数据。 ii. 解析 cell_codes 数组中的原始电池电压数据：将存储在 cell_codes
数组里的原始电池电压数据进行处理，转换为可使用的形式。 iii.
对比读回数据的奇偶校验码（PEC）和每个读寄存器命令计算得出的
PEC：验证读回数据的准确性，确保数据在传输过程中没有出错。 b. 寄存器值（Reg）不等于 0： i.
读取菊链中所有集成电路（IC）的单个电池电压寄存器：针对菊链中的每一个
IC，读取指定的单个电池电压寄存器的数据。 ii. 解析 cell_codes
数组中的原始电池电压数据：和前面一样，对 cell_codes 数组里的原始数据进行处理。 iii.
对比读回数据的奇偶校验码（PEC）和每个读寄存器命令计算得出的 PEC：检查数据的完整性和准确性。
2. 返回奇偶校验错误（pec_error）标志
最后返回一个表示是否存在奇偶校验错误的标志，用于告知调用该序列的程序数据是否存在错误。
*/

/**
 * @brief   从 LTC6804 电池电压寄存器读取原始数据
 *          该函数用于读取单个电池电压寄存器，并将读取到的数据以字节数组的形式存储在 *data
 *          指针所指向的位置。 此函数除了在 LTC6804_rdcv() 命令中使用外，很少会在其他地方被调用。
 * @param   reg:此参数用于控制读取哪个电池电压寄存器。
 *              0：读取所有电池寄存器。
 *              1：读取电池组 A。
 *              2：读取电池组 B。
 *              3：读取电池组 C。
 *              4：读取电池组 D。
 * @param   total_ic:这是菊链中集成电路（IC）的数量（仅取 -1）。
 * @param   *data:一个用于存储未解析电池编码的数组。
 *                函数执行后，会将读取到的原始电池编码存放在该数组中。
 */
void LTC6804_rdcv_reg(uint8_t reg, uint8_t total_ic, uint8_t* data) {
   const uint8_t REG_LEN = 8;  // number of bytes in each ICs register + 2 bytes for the PEC
   uint8_t cmd[4];
   uint16_t cmd_pec;

   // 1
   if (reg == 1) {  // 1: RDCVA
      cmd[1] = 0x04;
      cmd[0] = 0x00;
   } else if (reg == 2) {  // 2: RDCVB
      cmd[1] = 0x06;
      cmd[0] = 0x00;
   } else if (reg == 3) {  // 3: RDCVC
      cmd[1] = 0x08;
      cmd[0] = 0x00;
   } else if (reg == 4) {  // 4: RDCVD
      cmd[1] = 0x0A;
      cmd[0] = 0x00;
   }

   // 2
   cmd_pec = pec15_calc(2, cmd);
   cmd[2] = (uint8_t)(cmd_pec >> 8);
   cmd[3] = (uint8_t)(cmd_pec);

   // 3
   wakeup_idle();  // This will guarantee that the LTC6804 isoSPI port is awake. This command can be
                   // removed.

   // 4
   CS_LTC6804(0);
   spi_write_read(cmd, 4, data, (REG_LEN * total_ic));
   CS_LTC6804(1);
}
/*
### LTC6804_rdcv_reg 函数流程：
1.
**确定命令并初始化命令数组**：明确要执行的具体命令，并对存储命令的数组进行初始化操作，为后续的命令发送做准备。
2.
**计算命令的奇偶校验码（PEC）**：对确定好的命令计算其奇偶校验码，用于在数据传输过程中进行错误检测，保证数据的准确性。
3.
**唤醒隔离串行外设接口（isoSPI），此步骤可选**：如果系统中的 isoSPI
处于休眠状态，可通过此步骤将其唤醒，以便后续进行数据通信。不过该步骤并非必需，需根据实际的系统状态和需求来决定是否执行。
4.
**向 LTC6804 菊链发送全局命令**：将之前确定并处理好的命令发送给 LTC6804
芯片组成的菊链，以实现对这些芯片的统一控制和数据交互。
*/

/***********************************************************************************/ /**
 \brief Reads and parses the LTC6804 auxiliary registers.

 The function is used
 to read the  parsed GPIO codes of the LTC6804. This function will send the requested
 read commands parse the data and store the gpio voltages in aux_codes variable

 @param[in] uint8_t reg; This controls which GPIO voltage register is read back.

 0: Read back all auxiliary registers

 1: Read back auxiliary group A

 2: Read back auxiliary group B


 @param[in] uint8_t total_ic; This is the number of ICs in the daisy chain(-1 only)


 @param[out] uint16_t aux_codes[][6]; A two dimensional array of the gpio voltage codes. The GPIO
 codes will be stored in the aux_codes[][6] array in the following format: |  aux_codes[0][0]|
 aux_codes[0][1] |  aux_codes[0][2]|  aux_codes[0][3]|  aux_codes[0][4]|  aux_codes[0][5]|
 aux_codes[1][0] |aux_codes[1][1]|  .....    |
 |-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|---------------|-----------|
 |IC1 GPIO1        |IC1 GPIO2        |IC1 GPIO3        |IC1 GPIO4        |IC1 GPIO5        |IC1
 Vref2        |IC2 GPIO1        |IC2 GPIO2      |  .....    |

 @return  int8_t, PEC Status

 0: No PEC error detected

 -1: PEC error detected, retry read
 *************************************************/
/**
 * @brief   读取并解析 LTC6804 辅助寄存器
            该函数用于读取 LTC6804 经解析后的通用输入输出（GPIO）编码。
            此函数会发送所需的读取命令，对返回的数据进行解析，并将 GPIO 电压值存储在
            aux_codes变量中。
 */
int8_t LTC6804_rdaux(uint8_t reg, uint8_t total_ic, uint16_t aux_codes[][6]) {
   const uint8_t NUM_RX_BYT = 8;
   const uint8_t BYT_IN_REG = 6;
   const uint8_t GPIO_IN_REG = 3;

   uint8_t* data;
   uint8_t data_counter = 0;
   int8_t pec_error = 0;
   uint16_t parsed_aux;
   uint16_t received_pec;
   uint16_t data_pec;
   data = (uint8_t*)malloc((NUM_RX_BYT * total_ic) * sizeof(uint8_t));
   // 1.a
   if (reg == 0) {
      // a.i
      for (uint8_t gpio_reg = 1; gpio_reg < 3; gpio_reg++) {
         // executes once for each of the LTC6804 aux voltage registers
         data_counter = 0;
         LTC6804_rdaux_reg(gpio_reg, total_ic, data);
         // Reads the raw auxiliary register data into the data[] array

         for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
            // executes for every LTC6804 in the daisy chain current_ic is used as the IC counter
            // a.ii
            for (uint8_t current_gpio = 0; current_gpio < GPIO_IN_REG; current_gpio++) {
               // This loop parses the read back data into GPIO voltages, it loops once for each of
               // the 3 gpio voltage codes in the register
               parsed_aux = data[data_counter] + (data[data_counter + 1] << 8);
               // Each gpio codes is received as two bytes and is combined to
               //  create the parsed gpio voltage code

               aux_codes[current_ic][current_gpio + ((gpio_reg - 1) * GPIO_IN_REG)] = parsed_aux;
               data_counter = data_counter + 2;
               // Because gpio voltage codes are two bytes the data counter
               // must increment by two for each parsed gpio voltage code
            }
            // a.iii
            received_pec = (data[data_counter] << 8) + data[data_counter + 1];
            // The received PEC for the current_ic is transmitted as
            // the 7th and 8th after the 6 gpio voltage data bytes
            data_pec = pec15_calc(BYT_IN_REG, &data[current_ic * NUM_RX_BYT]);
            if (received_pec != data_pec) {
               pec_error = 1;
               // The pec_error variable is simply set negative if any PEC errors
               // are detected in the received serial data
            }

            data_counter = data_counter + 2;
            // Because the transmitted PEC code is 2 bytes long the data_counter
            // must be incremented by 2 bytes to point to the next ICs gpio voltage data
         }
      }

   } else {
      // b.i
      LTC6804_rdaux_reg(reg, total_ic, data);
      for (int current_ic = 0; current_ic < total_ic; current_ic++) {
         // executes for every LTC6804 in the daisy chain
         // current_ic is used as an IC counter

         // b.ii
         for (int current_gpio = 0; current_gpio < GPIO_IN_REG; current_gpio++) {
            // This loop parses the read back data. Loops
            // once for each aux voltage in the register

            parsed_aux = (data[data_counter] + (data[data_counter + 1] << 8));
            // Each gpio codes is received as two bytes and is combined to
            //  create the parsed gpio voltage code
            aux_codes[current_ic][current_gpio + ((reg - 1) * GPIO_IN_REG)] = parsed_aux;
            data_counter = data_counter + 2;
            // Because gpio voltage codes are two bytes the data counter
            // must increment by two for each parsed gpio voltage code
         }
         // b.iii
         received_pec = (data[data_counter] << 8) + data[data_counter + 1];
         // The received PEC for the current_ic is transmitted as the
         // 7th and 8th after the 6 gpio voltage data bytes
         data_pec = pec15_calc(BYT_IN_REG, &data[current_ic * NUM_RX_BYT]);
         if (received_pec != data_pec) {
            pec_error = 1;
            // The pec_error variable is simply set negative if any PEC errors
            // are detected in the received serial data
         }

         data_counter = data_counter + 2;
         // Because the transmitted PEC code is 2 bytes long the data_counter
         // must be incremented by 2 bytes to point to the next ICs gpio voltage data
      }
   }
   free(data);
   return (pec_error);
}
/*
LTC6804_rdaux Sequence

1. Switch Statement:
a. Reg = 0
i. Read GPIO voltage registers A-D for every IC in the daisy chain
ii. Parse raw GPIO voltage data in cell_codes array
iii. Check the PEC of the data read back vs the calculated PEC for each read register command
b. Reg != 0
i.Read single GPIO voltage register for all ICs in daisy chain
ii. Parse raw GPIO voltage data in cell_codes array
iii. Check the PEC of the data read back vs the calculated PEC for each read register command
2. Return pec_error flag
*/

/***********************************************/ /**
 \brief Read the raw data from the LTC6804 auxiliary register

 The function reads a single GPIO voltage register and stores thre read data
 in the *data point as a byte array. This function is rarely used outside of
 the LTC6804_rdaux() command.

 @param[in] uint8_t reg; This controls which GPIO voltage register is read back.

 1: Read back auxiliary group A

 2: Read back auxiliary group B


 @param[in] uint8_t total_ic; This is the number of ICs in the daisy chain

 @param[out] uint8_t *data; An array of the unparsed aux codes



 Command Code:
 -------------

 |CMD[0:1]	    |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6 |
 5   |   4   |   3   |   2   |   1   |   0   |
 |---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
 |RDAUXA:	    |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0 |
 0   |   0   |   1   |   1   |   0   |   0   | |RDAUXB:	    |   0   |   0   |   0   |   0   |   0 |
 0   |   0   |   0   |   0   |   0   |   0   |   0   |   1   |   1   |   1   |   0   |

 *************************************************/
/**
 * @brief   从 LTC6804 辅助寄存器读取原始数据
            该函数用于读取单个通用输入输出（GPIO）电压寄存器，并将读取到的数据以字节数组的形式存储在
            data 指针所指向的位置。 除了在 LTC6804_rdaux()
            命令中使用外，该函数很少会在其他地方被调用。
 */
void LTC6804_rdaux_reg(uint8_t reg, uint8_t total_ic, uint8_t* data) {
   const uint8_t REG_LEN = 8;  // number of bytes in the register + 2 bytes for the PEC
   uint8_t cmd[4];
   uint16_t cmd_pec;

   // 1
   if (reg == 1) {  // Read back auxiliary group A
      cmd[1] = 0x0C;
      cmd[0] = 0x00;
   } else if (reg == 2) {  // Read back auxiliary group B
      cmd[1] = 0x0e;
      cmd[0] = 0x00;
   } else {
      cmd[1] = 0x0C;
      cmd[0] = 0x00;
   }  // Read back auxiliary group A
   // 2
   cmd_pec = pec15_calc(2, cmd);
   cmd[2] = (uint8_t)(cmd_pec >> 8);
   cmd[3] = (uint8_t)(cmd_pec);

   // 3
   wakeup_idle();  // This will guarantee that the LTC6804 isoSPI port is awake, this command can be
                   // removed.
   // 4
   CS_LTC6804(0);
   spi_write_read(cmd, 4, data, (REG_LEN * total_ic));
   CS_LTC6804(1);
}
/*
LTC6804_rdaux_reg Function Process:
1. Determine Command and initialize command array
2. Calculate Command PEC
3. Wake up isoSPI, this step is optional
4. Send Global Command to LTC6804 daisy chain
*/

/********************************************************/ /**
 \brief Clears the LTC6804 cell voltage registers

 The command clears the cell voltage registers and intiallizes
 all values to 1. The register will read back hexadecimal 0xFF
 after the command is sent.


 Command Code:
 -------------

 |CMD[0:1]	    |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6 |
 5   |   4   |   3   |   2   |   1   |   0   |
 |---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
 |CLRCELL:	    |   0   |   0   |   0   |   0   |   0   |   1   |   1   |   1   |   0   |   0 |
 0   |   1   |   0   |   0   |   0   |   1   |
 ************************************************************/
/**
 * @brief   清空 LTC6804 电池电压寄存器
            此命令用于清空电池电压寄存器，并将所有值初始化为 1。
            在发送该命令后，寄存器读回的值将为十六进制的 0xFF。
 */
void LTC6804_clrcell() {
   uint8_t cmd[4];
   uint16_t cmd_pec;

   // 1
   cmd[0] = 0x07;
   cmd[1] = 0x11;

   // 2
   cmd_pec = pec15_calc(2, cmd);
   cmd[2] = (uint8_t)(cmd_pec >> 8);
   cmd[3] = (uint8_t)(cmd_pec);

   // 3
   wakeup_idle();  // This will guarantee that the LTC6804 isoSPI port is awake. This command can be
                   // removed.

   // 4
   CS_LTC6804(0);
   spi_write_read(cmd, 4, 0, 0);
   CS_LTC6804(1);
}
/*
LTC6804_clrcell Function sequence:

1. Load clrcell command into cmd array
2. Calculate clrcell cmd PEC and load pec into cmd array
3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
4. send broadcast clrcell command to LTC6804 daisy chain
*/

/***********************************************************/ /**
 \brief Clears the LTC6804 Auxiliary registers

 The command clears the Auxiliary registers and intiallizes
 all values to 1. The register will read back hexadecimal 0xFF
 after the command is sent.


 Command Code:
 -------------

 |CMD[0:1]	    |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6 |
 5   |   4   |   3   |   2   |   1   |   0   |
 |---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
 |CLRAUX:	    |   0   |   0   |   0   |   0   |   0   |   1   |   1   |   1   |   0   |   0 |
 0   |   1   |   0   |   0   |   2   |   0   |
 ***************************************************************/
/**
 * @brief   清空 LTC6804 辅助寄存器
            该命令用于清空 LTC6804 的辅助寄存器，并将所有值初始化为 1。
            在发送此命令后，寄存器读取返回的结果将是十六进制的 0xFF。
 */
void LTC6804_clraux() {
   uint8_t cmd[4];
   uint16_t cmd_pec;

   // 1
   cmd[0] = 0x07;
   cmd[1] = 0x12;

   // 2
   cmd_pec = pec15_calc(2, cmd);
   cmd[2] = (uint8_t)(cmd_pec >> 8);
   cmd[3] = (uint8_t)(cmd_pec);

   // 3
   wakeup_idle();  // This will guarantee that the LTC6804 isoSPI port is awake.This command can be
                   // removed.
   // 4
   CS_LTC6804(0);
   spi_write_read(cmd, 4, 0, 0);
   CS_LTC6804(1);
}
/*
LTC6804_clraux Function sequence:

1. Load clraux command into cmd array
2. Calculate clraux cmd PEC and load pec into cmd array
3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
4. send broadcast clraux command to LTC6804 daisy chain
*/

/*****************************************************/ /**
 \brief Write the LTC6804 configuration register

 This command will write the configuration registers of the LTC6804-1s
 connected in a daisy chain stack. The configuration is written in descending
 order so the last device's configuration is written first.

 @param[in] uint8_t total_ic; The number of ICs being written to.

 @param[in] uint8_t config[][6] is a two dimensional array of the configuration data that will be
 written, the array should contain the 6 bytes for each IC in the daisy chain. The lowest IC in the
 daisy chain should be the first 6 byte block in the array. The array should have the following
 format: |  config[0][0]| config[0][1] |  config[0][2]|  config[0][3]|  config[0][4]|  config[0][5]|
 config[1][0] |  config[1][1]|  config[1][2]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
 |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2
 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |

 The function will calculate the needed PEC codes for the write data
 and then transmit data to the ICs on a daisy chain.


 Command Code:
 -------------
 |               |							CMD[0] | CMD[1] |
 |---------------|---------------------------------------------------------------|---------------------------------------------------------------|
 |CMD[0:1]	    |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6 |
 5   |   4   |   3   |   2   |   1   |   0   |
 |---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
 |WRCFG:	        |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   | 0
 |   0   |   0   |   0   |   0   |   0   |   1   |
 ********************************************************/
/**
 * @brief   写入 LTC6804 配置寄存器
            此命令用于对以菊链方式连接的 LTC6804 - 1 芯片的配置寄存器进行写入操作。
            配置信息会按照降序顺序写入，也就是说，会先写入最后一个设备的配置信息。
 */
void LTC6804_wrcfg(uint8_t total_ic, uint8_t config[][6]) {
   const uint8_t BYTES_IN_REG = 6;
   const uint8_t CMD_LEN = 4 + (8 * total_ic);
   uint8_t* cmd;
   uint16_t cfg_pec;
   uint8_t cmd_index;  // command counter

   cmd = (uint8_t*)malloc(CMD_LEN * sizeof(uint8_t));

   // 1
   cmd[0] = 0x00;
   cmd[1] = 0x01;
   cmd[2] = 0x3d;
   cmd[3] = 0x6e;

   // 2
   cmd_index = 4;
   for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--) {
      // executes for each LTC6804 in daisy chain, this loops starts with
      // the last IC on the stack. The first configuration written is
      // received by the last IC in the daisy chain

      for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) {
         // executes for each of the 6 bytes in the CFGR register
         // current_byte is the byte counter

         cmd[cmd_index] = config[current_ic - 1][current_byte];
         // adding the config data to the array to be sent
         cmd_index = cmd_index + 1;
      }
      // 3
      cfg_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &config[current_ic - 1][0]);
      // calculating the PEC for each ICs configuration register data
      cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
      cmd[cmd_index + 1] = (uint8_t)cfg_pec;
      cmd_index = cmd_index + 2;
   }

   // 4
   wakeup_idle();  // This will guarantee that the LTC6804 isoSPI port is awake.This command can be
                   // removed.
   // 5
   CS_LTC6804(0);
   spi_write_array(CMD_LEN, cmd);
   CS_LTC6804(1);
   free(cmd);
}
/*
WRCFG Sequence:

1. Load cmd array with the write configuration command and PEC
2. Load the cmd with LTC6804 configuration data
3. Calculate the pec for the LTC6804 configuration data being transmitted
4. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
5. Write configuration data to the LTC6804 daisy chain

*/

/*!******************************************************
\brief Reads configuration registers of a LTC6804 daisy chain

@param[in] uint8_t total_ic: number of ICs in the daisy chain

@param[out] uint8_t r_config[][8] is a two dimensional array that the function stores the read
configuration data. The configuration data for each IC is stored in blocks of 8 bytes with the
configuration data of the lowest IC on the stack in the first 8 bytes block of the array, the second
IC in the second 8 byte etc. Below is an table illustrating the array organization:

|r_config[0][0]|r_config[0][1]|r_config[0][2]|r_config[0][3]|r_config[0][4]|r_config[0][5]|r_config[0][6]
|r_config[0][7] |r_config[1][0]|r_config[1][1]|  .....    |
|--------------|--------------|--------------|--------------|--------------|--------------|----------------|---------------|--------------|--------------|-----------|
|IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC1 PEC
High    |IC1 PEC Low    |IC2 CFGR0     |IC2 CFGR1     |  .....    |


@return int8_t, PEC Status.

0: Data read back has matching PEC

-1: Data read back has incorrect PEC


Command Code:
-------------

|CMD[0:1]		|  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   | 6
|   5   |   4   |   3   |   2   |   1   |   0   |
|---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|RDCFG:	        |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   | 0
|   1   |   0   |   0   |   1   |   0   |
********************************************************/
/**
 * @brief   读取 LTC6804 菊链的配置寄存器
 */
int8_t LTC6804_rdcfg(uint8_t total_ic, uint8_t r_config[][8]) {
   const uint8_t BYTES_IN_REG = 8;

   uint8_t cmd[4];
   uint8_t* rx_data;
   int8_t pec_error = 0;
   uint16_t data_pec;
   uint16_t received_pec;

   rx_data = (uint8_t*)malloc((8 * total_ic) * sizeof(uint8_t));

   // 1
   cmd[0] = 0x00;
   cmd[1] = 0x02;
   cmd[2] = 0x2b;
   cmd[3] = 0x0A;

   // 2
   wakeup_idle();  // This will guarantee that the LTC6804 isoSPI port is awake. This command can be
                   // removed.
   // 3
   CS_LTC6804(0);
   spi_write_read(cmd, 4, rx_data, (BYTES_IN_REG * total_ic));
   // Read the configuration data of all ICs on the daisy chain into
   CS_LTC6804(1);  // rx_data[] array

   for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
      // executes for each LTC6804 in the daisy chain and packs the data
      // into the r_config array as well as check the received Config data
      // for any bit errors
      // 4.a
      for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) {
         r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic * BYTES_IN_REG)];
      }
      // 4.b
      received_pec = (r_config[current_ic][6] << 8) + r_config[current_ic][7];
      data_pec = pec15_calc(6, &r_config[current_ic][0]);
      if (received_pec != data_pec) {
         pec_error = 1;
      }
   }

   free(rx_data);
   // 5
   return (pec_error);
}
/*
RDCFG Sequence:

1. Load cmd array with the write configuration command and PEC
2. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
3. Send command and read back configuration data
4. For each LTC6804 in the daisy chain
a. load configuration data into r_config array
b. calculate PEC of received data and compare against calculated PEC
5. Return PEC Error

*/

/*!****************************************************
\brief Wake isoSPI up from idle state
Generic wakeup commannd to wake isoSPI up out of idle
*****************************************************/
/**
 * @brief   使 isoSPI 从空闲状态唤醒
            这是一个通用的唤醒命令，用于将 isoSPI（隔离串行外设接口）从空闲状态中唤醒。
 */
void wakeup_idle() {
   CS_LTC6804(0);
   delay_ms(3);  // Guarantees the isoSPI will be in ready mode
   CS_LTC6804(1);
}

/*!****************************************************
\brief Wake the LTC6804 from the sleep state

Generic wakeup commannd to wake the LTC6804 from sleep
*****************************************************/
/**
 * @brief   唤醒处于睡眠状态的 LTC6804
            这是一个通用的唤醒命令，用于将 LTC6804 从睡眠状态中唤醒。
 */
void wakeup_sleep() {
   CS_LTC6804(0);
   delay_ms(3);  // Guarantees the LTC6804 will be in standby
   CS_LTC6804(1);
}
/*!**********************************************************
\brief calaculates  and returns the CRC15

@param[in] uint8_t len: the length of the data array being passed to the function

@param[in] uint8_t data[] : the array of data that the PEC will be generated from


@returns The calculated pec15 as an unsigned int
***********************************************************/
/**
 * @brief   计算并返回 CRC15 校验值
 */
uint16_t pec15_calc(uint8_t len, uint8_t* data) {
   uint16_t remainder, addr;

   remainder = 16;                    // initialize the PEC
   for (uint8_t i = 0; i < len; i++)  // loops for each byte in data array
   {
      addr = ((remainder >> 7) ^ data[i]) & 0xff;  // calculate PEC table address
      remainder = (remainder << 8) ^ crc15Table[addr];
   }
   return (remainder * 2);  // The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}

/*!
\brief Writes an array of bytes out of the SPI port

@param[in] uint8_t len length of the data array being written on the SPI port
@param[in] uint8_t data[] the data array to be written on the SPI port

*/
/**
 * @brief   从串行外设接口（SPI）端口输出一个字节数组。
 */
void spi_write_array(uint8_t len, uint8_t data[]) {
   for (uint8_t i = 0; i < len; i++) {
      // spi_write((int8_t)data[i]);---原版函数
      spi1_r_w_byte((uint8_t)data[i]);
   }
}

/*!
\brief Writes and read a set number of bytes using the SPI port.

@param[in] uint8_t tx_data[] array of data to be written on the SPI port
@param[in] uint8_t tx_len length of the tx_data array
@param[out] uint8_t rx_data array that read data will be written too.
@param[in] uint8_t rx_len number of bytes to be read from the SPI port.

*/
/**
 * @brief   使用串行外设接口（SPI）端口写入并读取一定数量的字节。
 */
void spi_write_read(uint8_t tx_Data[], uint8_t tx_len, uint8_t* rx_data, uint8_t rx_len) {
   for (uint8_t i = 0; i < tx_len; i++) {
      // spi_write(tx_Data[i]);
      spi1_r_w_byte(tx_Data[i]);
   }

   for (uint8_t i = 0; i < rx_len; i++) {
      // rx_data[i] = (uint8_t)spi_read(0xFF);
      rx_data[i] = (uint8_t)spi1_r_w_byte(0xFF);
   }
}

// 手册第49页
/* 寄存器               8         7          6         5         4         3         2        1 */
// CFGR0       RD/WR   GPIO5     GPIO4      GPIO3     GPIO2     GPIO1     REFON     SWTRD    ADCOPT
// CFGR1       RD/WR   VUV[7]    VUV[6]     VUV[5]    VUV[4]    VUV[3]    VUV[2]    VUV[1]   VUV[0]
// CFGR2       RD/WR   VOV[3]    VOV[2]     VOV[1]    VOV[0]    VUV[11]   VUV[10]   VUV[9]   VUV[8]
// CFGR3       RD/WR   VOV[11]   VOV[10]    VOV[9]    VOV[8]    VOV[7]    VOV[6]    VOV[5]   VOV[4]
// CFGR4       RD/WR   DCC8      DCC7       DCC6      DCC5      DCC4      DCC3      DCC2     DCC1
// CFGR5       RD/WR   DCTO[3]   DCTO[2]    DCTO[1]   DCTO[0]   DCC12     DCC11     DCC10    DCC9
/*
void init_cfg(void)
{
    int i;

    for(i = 0; i<TOTAL_IC;i++)
    {
        tx_cfg[i][0] = 0xFE ;   //GPIO引脚下拉电路关断（bit8~bit4） | 基准保持上电状态（bit3） |
SWTEN处于逻辑1（软件定时器） | ADC模式选择为0 tx_cfg[i][1] = 0x00 ;   //不使用欠压比较功能
        tx_cfg[i][2] = 0x00 ;   //不使用过压比较功能
        tx_cfg[i][3] = 0x00 ;
        tx_cfg[i][4] = 0x00 ;   //不使用电池放电功能
        tx_cfg[i][5] = 0x00 ;   //放电超时时间
    }
}
*/
