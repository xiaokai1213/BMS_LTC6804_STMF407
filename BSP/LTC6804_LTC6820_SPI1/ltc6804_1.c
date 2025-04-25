#include "./BSP/LTC6804_LTC6820_SPI1/ltc6804_1.h"
/*
uint8_t tx_cfg[6][6];
uint8_t rx_cfg[6][8];
static uint8_t TOTAL_IC = 1;

void init_cfg(void) {
   int i;
   for (i = 0; i < TOTAL_IC; i++) {
      tx_cfg[i][0] = 0xFE;  // GPIO����������·�ضϣ�bit8~bit4�� | ��׼�����ϵ�״̬��bit3�� |
                            // SWTEN�����߼�1�������ʱ���� | ADCģʽѡ��Ϊ0
      tx_cfg[i][1] = 0x00;  // ��ʹ��Ƿѹ�ȽϹ���
      tx_cfg[i][2] = 0x00;  // ��ʹ�ù�ѹ�ȽϹ���
      tx_cfg[i][3] = 0x00;
      tx_cfg[i][4] = 0x00;  // ��ʹ�õ�طŵ繦��
      tx_cfg[i][5] = 0xF0;  // ����ŵ糬ʱʱ��120min
   }
}
*/
/* 6804 ת��������� */
uint8_t ADCV[2];  // ��ص�ѹת������
uint8_t ADAX[2];  // ͨ�����������GPIO��ת������

/**
 * @brief   ��ʼ��LTC6804��ƬѡIO,����ʼ��SPI1����
 * @note    �Լ�6804����
 * @param   ��
 * @retval  ��
 */
void LTC6804_init(void) {
   __HAL_RCC_GPIOB_CLK_ENABLE();                         // 6804Ƭѡ��ʱ��ʹ��
   GPIO_InitTypeDef gpio_init_struct;                    // ����gpio��ʼ���ṹ��
   gpio_init_struct.Pin = CS_6804_GPIO_PIN;              // cs6804���ţ�PB6��
   gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;          // �������
   gpio_init_struct.Pull = GPIO_PULLUP;                  // ��������
   gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        // ���Ÿ���ģʽ
   HAL_GPIO_Init(CS_6804_GPIO_PORT, &gpio_init_struct);  // ����CS��gpio
   CS_LTC6804(1);                                        // ȡ��Ƭѡ

   spi1_init();  // ��ʼ��SPI1
   // spi1_set_speed(SPI_SPEED_128);  // ����SPI1���ٶ�

   // init_cfg();                                                 // ����LTC6804�ļĴ���
   uint8_t tx_cfg[6][6];
   uint8_t rx_cfg[6][8];
   uint8_t TOTAL_IC = 1;
   int i;
   for (i = 0; i < TOTAL_IC; i++) {
      tx_cfg[i][0] = 0xFE;  // GPIO����������·�ضϣ�bit8~bit4�� | ��׼�����ϵ�״̬��bit3�� |
      tx_cfg[i][1] = 0x00;  // SWTEN�����߼�1�������ʱ���� | ADCģʽѡ��Ϊ0
      tx_cfg[i][2] = 0x00;  // ��ʹ��Ƿѹ�ȽϹ���
      tx_cfg[i][3] = 0x00;  // ��ʹ�ù�ѹ�ȽϹ���
      tx_cfg[i][4] = 0x00;  // ��ʹ�õ�طŵ繦��
      tx_cfg[i][5] = 0xF0;  // ����ŵ糬ʱʱ��120min
   }
   set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL);  // ����adcת������
   wakeup_sleep();                                             // ����оƬ
   LTC6804_adcv();                                             //
   LTC6804_wrcfg(TOTAL_IC, tx_cfg);                            // �����������д��оƬ
   wakeup_idle();
   wakeup_idle();                               // ��ֹ���ݵ�һ��byte��Ϊ255��REFON=1
   if (LTC6804_rdcfg(TOTAL_IC, rx_cfg) == 1) {  // ���һ�µ�����û�����óɹ�
      printf("LTC6804_MODULAR INIT NG!\n\r");
   } else {
      printf("LTC6804_MODULAR INIT OK!\n\r");
   }
}

/**
 * @brief  ��ȫ�� ADC ���Ʊ���ӳ�䵽ÿ����ͬ ADC �����Ӧ���ʵ������ֽڡ�
 * @param   MD: ADC ת��ģʽ
 * @param   DCP�������ڵ�ص�Ԫת���ڼ��Ƿ�����ŵ�
 * @param   CH��ȷ���� ADC ת������ִ���ڼ������Щ��ص�Ԫ
 * @param   CHG��ȷ���ڸ���ת������ִ���ڼ������Щͨ�����������GPIO��ͨ��
 * @retval  ��
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
 * @brief   ������ص�ѹת����
 *          ���� LTC6804 �� C ���������ģ����ADC��ת����
 *          ����ͨ��������ص�ȫ�ֱ�����������ִ�е� ADC ת�����͡�
 * @param   ��
 * @retval  ��
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
   // �⽫ȷ�� LTC6804 �ĸ���ʽ��������ӿڣ�isoSPI���˿ڴ��ڻ���״̬�����������ɾ����

   // 4
   CS_LTC6804(0);            // ����Ƭѡ
   spi_write_array(4, cmd);  //
   CS_LTC6804(1);            // ����ȡ��Ƭѡ
}
/*
LTC6804_adcv �������У�
�� adcv ������ص����������С�
���� adcv �������żУ���루PEC������������ص����������С�
���Ѹ��� SPI��isoSPI���˿ڡ������ǰ��ȷ�� isoSPI ��״̬����˲������ʡ�ԡ�
�� LTC6804 �������͹㲥 adcv ���
*/

/**
 * @brief   ����һ��ͨ�����������GPIO��ת��
 *          ���� LTC6804 �� GPIO �����ģ����ADC��ת����
 *          ��ͨ��������ص�ȫ�ֱ�����������ִ�е� ADC ת������
 * @param   ��
 * @retval  ��
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
   // �⽫ȷ�� LTC6804 �ĸ���ʽ��������ӿڣ�isoSPI���˿ڴ��ڻ���״̬���������ǿ���ȥ���ġ�

   CS_LTC6804(0);            // ����Ƭѡ
   spi_write_array(4, cmd);  //
   CS_LTC6804(1);            // ����ȡ��Ƭѡ
}
/*
LTC6804_adax ����ִ�в��裺
�� adax ���������������顣
���� adax �����ѭ������У���루PEC���������������������顣
���Ѹ���ʽ��������ӿڣ�isoSPI���˿ڡ�����ǰ��ȷ�� isoSPI �˿�״̬�������˲����ʡ�ԡ�
�� LTC6804 �������͹㲥 adax ���
*/

/**
 * @brief   ��ȡ������ LTC6804 �ĵ�ص�ѹ�Ĵ�����
 *          �˺������ڶ�ȡ LTC6804 �ĵ�ر��롣
 *          �ú����ᷢ������Ķ�ȡ����������ݣ�������ص�ѹ�洢��cell_codes�����С�
 * @param   reg:�˲������ڿ��ƶ�ȡ�ĸ���ص�ѹ�Ĵ�����
 *              0����ȡ���е�ؼĴ�����
 *              1����ȡ����� A��
 *              2����ȡ����� B��
 *              3����ȡ����� C��
 *              4����ȡ����� D��
 * @param   total_ic:���Ǿ����м��ɵ�·��IC������������ȡ -1����
 * @param   cell_codes[][12]:����һ���洢������ĵ�ر�������飬���ӵ͵��ߵ�˳�����С�
 *              ��ر��뽫�����¸�ʽ�洢�� cell_codes[] �����У�
 *              cell_codes[i][j]:i��ʾIC������j��ʾ����IC�ĵ����0-12
 * @retval  ��żУ���루PEC��״̬��
 *          0��δ��⵽ PEC ����
 *          1����⵽ PEC ���������Զ�ȡ��
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
   uint8_t data_counter = 0;  // ���ݼ�����
   cell_data = (uint8_t*)malloc((NUM_RX_BYT * total_ic) * sizeof(uint8_t));
   // 1.a
   if (reg == 0) {
      // a.i
      for (uint8_t cell_reg = 1; cell_reg < 5; cell_reg++) {
         // ���� LTC6804 ��ÿ����ص�ѹ�Ĵ�����ִ��һ�Ρ�
         data_counter = 0;
         LTC6804_rdcv_reg(cell_reg, total_ic, cell_data);  // ��ȡ������ص�ѹ�Ĵ���

         for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
            // �Ծ����е�ÿһ��LTC6804��ִ�У��ò���������ǰ��
            // ��current_ic�����������ɵ�·��IC����������
            // a.ii
            for (uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++) {
               // ���ѭ���Ѷ��ص����ݽ����ɵ�ص�ѹ����ԼĴ������ 3 ����ص�ѹ�����ִ��һ�Ρ�
               parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
               // ÿ����ر����������ֽڵ���ʽ���գ�Ȼ���������ֽ���������������ɽ�����ĵ�ص�ѹ���롣

               cell_codes[current_ic][current_cell + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
               data_counter = data_counter + 2;
               // ���ڵ�ص�ѹ�����������ֽڣ���˶���ÿ���ѽ����ĵ�ر��룬���ݼ������������ 2��
            }
            // a.iii
            received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter + 1];
            // ��ǰ���ɵ�·��IC�����յ���ѭ������У���루PEC������ 6 ����ص�ѹ�����ֽ�֮���Ե� 7
            // ���͵� 8 ���ֽڵ���ʽ���д��䡣
            data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
            if (received_pec != data_pec) {
               pec_error = 1;
               // ����ڴ��������м�⵽�κ���żУ�����PEC ���󣩣�`pec_error`
               // �����ͻᱻ�򵥵�����Ϊ��ֵ��
            }
            data_counter = data_counter + 2;
            // ���ڴ������żУ���루PEC��Ϊ 2 �ֽڳ���������ݼ������������ 2
            // �ֽڣ���ָ��������ļ��ɵ�·��IC���ĵ�ص�ѹ���ݡ�
         }
      }
   }
   // 1.b
   else {
      // b.i
      LTC6804_rdcv_reg(reg, total_ic, cell_data);
      for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) {
         // ���ھ����е�ÿ�� LTC6804 ����ִ�У���Ӧ��������`current_ic`�������ɵ�·��IC����������
         // b.ii
         for (uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++) {
            // ���ѭ�������ص����ݽ���Ϊ��ص�ѹ�����ڼĴ����е�������ص�ѹ�����е�ÿһ����������ѭ��һ�Ρ�

            parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
            // ÿ����ر����������ֽڵ���ʽ���գ�Ȼ���������ֽںϲ����Ӷ����ɽ�����ĵ�ص�ѹ���롣

            cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)] =
                0x0000FFFF & parsed_cell;
            data_counter = data_counter + 2;
            // ��Ϊ��ص�ѹ�����������ֽڣ����Զ���ÿһ���ѽ����ĵ�ر��룬���ݼ��������������� 2 ��
         }
         // b.iii
         received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter + 1];
         // ��ǰ���ɵ�·��current_ic�������յ�����żУ���루PEC������ 6
         // ����ص�ѹ�����ֽ�֮����Ϊ�� 7 �͵� 8 ���ֽڽ��д��䡣
         data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
         if (received_pec != data_pec) {
            pec_error = 1;
            // ����ڴ��������м�⵽�κ���żУ���루PEC��������ô��pec_error�������ͻ�򵥵ر�����Ϊ��ֵ��
         }
         data_counter = data_counter + 2;
         // ���ڴ������żУ���루PEC������Ϊ2���ֽڣ�������ݼ������������2���ֽڣ��Ա�ָ��������ļ��ɵ�·��IC���ĵ�ص�ѹ���ݡ�
      }
   }
   // 2
   free(cell_data);
   return (pec_error);
}
/*
LTC6804_rdcv ����
1. ������䣺
a. �Ĵ���ֵ��Reg������ 0��
i. ��ȡ������ÿ�����ɵ�·��IC���ĵ�ص�ѹ�Ĵ��� A - D���Ծ������ÿһ�� IC����ȡ���ص�ѹ�Ĵ��� A
�� D �����ݡ� ii. ���� cell_codes �����е�ԭʼ��ص�ѹ���ݣ����洢�� cell_codes
�������ԭʼ��ص�ѹ���ݽ��д���ת��Ϊ��ʹ�õ���ʽ�� iii.
�Աȶ������ݵ���żУ���루PEC����ÿ�����Ĵ����������ó���
PEC����֤�������ݵ�׼ȷ�ԣ�ȷ�������ڴ��������û�г��� b. �Ĵ���ֵ��Reg�������� 0�� i.
��ȡ���������м��ɵ�·��IC���ĵ�����ص�ѹ�Ĵ�������Ծ����е�ÿһ��
IC����ȡָ���ĵ�����ص�ѹ�Ĵ��������ݡ� ii. ���� cell_codes
�����е�ԭʼ��ص�ѹ���ݣ���ǰ��һ������ cell_codes �������ԭʼ���ݽ��д��� iii.
�Աȶ������ݵ���żУ���루PEC����ÿ�����Ĵ����������ó��� PEC��������ݵ������Ժ�׼ȷ�ԡ�
2. ������żУ�����pec_error����־
��󷵻�һ����ʾ�Ƿ������żУ�����ı�־�����ڸ�֪���ø����еĳ��������Ƿ���ڴ���
*/

/**
 * @brief   �� LTC6804 ��ص�ѹ�Ĵ�����ȡԭʼ����
 *          �ú������ڶ�ȡ������ص�ѹ�Ĵ�����������ȡ�����������ֽ��������ʽ�洢�� *data
 *          ָ����ָ���λ�á� �˺��������� LTC6804_rdcv() ������ʹ���⣬���ٻ��������ط������á�
 * @param   reg:�˲������ڿ��ƶ�ȡ�ĸ���ص�ѹ�Ĵ�����
 *              0����ȡ���е�ؼĴ�����
 *              1����ȡ����� A��
 *              2����ȡ����� B��
 *              3����ȡ����� C��
 *              4����ȡ����� D��
 * @param   total_ic:���Ǿ����м��ɵ�·��IC������������ȡ -1����
 * @param   *data:һ�����ڴ洢δ������ر�������顣
 *                ����ִ�к󣬻Ὣ��ȡ����ԭʼ��ر������ڸ������С�
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
### LTC6804_rdcv_reg �������̣�
1.
**ȷ�������ʼ����������**����ȷҪִ�еľ���������Դ洢�����������г�ʼ��������Ϊ�������������׼����
2.
**�����������żУ���루PEC��**����ȷ���õ������������żУ���룬���������ݴ�������н��д����⣬��֤���ݵ�׼ȷ�ԡ�
3.
**���Ѹ��봮������ӿڣ�isoSPI�����˲����ѡ**�����ϵͳ�е� isoSPI
��������״̬����ͨ���˲��轫�份�ѣ��Ա������������ͨ�š������ò��貢�Ǳ��裬�����ʵ�ʵ�ϵͳ״̬�������������Ƿ�ִ�С�
4.
**�� LTC6804 ��������ȫ������**����֮ǰȷ��������õ�����͸� LTC6804
оƬ��ɵľ�������ʵ�ֶ���ЩоƬ��ͳһ���ƺ����ݽ�����
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
 * @brief   ��ȡ������ LTC6804 �����Ĵ���
            �ú������ڶ�ȡ LTC6804 ���������ͨ�����������GPIO�����롣
            �˺����ᷢ������Ķ�ȡ����Է��ص����ݽ��н��������� GPIO ��ѹֵ�洢��
            aux_codes�����С�
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
 * @brief   �� LTC6804 �����Ĵ�����ȡԭʼ����
            �ú������ڶ�ȡ����ͨ�����������GPIO����ѹ�Ĵ�����������ȡ�����������ֽ��������ʽ�洢��
            data ָ����ָ���λ�á� ������ LTC6804_rdaux()
            ������ʹ���⣬�ú������ٻ��������ط������á�
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
 * @brief   ��� LTC6804 ��ص�ѹ�Ĵ���
            ������������յ�ص�ѹ�Ĵ�������������ֵ��ʼ��Ϊ 1��
            �ڷ��͸�����󣬼Ĵ������ص�ֵ��Ϊʮ�����Ƶ� 0xFF��
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
 * @brief   ��� LTC6804 �����Ĵ���
            ������������� LTC6804 �ĸ����Ĵ�������������ֵ��ʼ��Ϊ 1��
            �ڷ��ʹ�����󣬼Ĵ�����ȡ���صĽ������ʮ�����Ƶ� 0xFF��
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
 * @brief   д�� LTC6804 ���üĴ���
            ���������ڶ��Ծ�����ʽ���ӵ� LTC6804 - 1 оƬ�����üĴ�������д�������
            ������Ϣ�ᰴ�ս���˳��д�룬Ҳ����˵������д�����һ���豸��������Ϣ��
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
 * @brief   ��ȡ LTC6804 ���������üĴ���
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
 * @brief   ʹ isoSPI �ӿ���״̬����
            ����һ��ͨ�õĻ���������ڽ� isoSPI�����봮������ӿڣ��ӿ���״̬�л��ѡ�
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
 * @brief   ���Ѵ���˯��״̬�� LTC6804
            ����һ��ͨ�õĻ���������ڽ� LTC6804 ��˯��״̬�л��ѡ�
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
 * @brief   ���㲢���� CRC15 У��ֵ
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
 * @brief   �Ӵ�������ӿڣ�SPI���˿����һ���ֽ����顣
 */
void spi_write_array(uint8_t len, uint8_t data[]) {
   for (uint8_t i = 0; i < len; i++) {
      // spi_write((int8_t)data[i]);---ԭ�溯��
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
 * @brief   ʹ�ô�������ӿڣ�SPI���˿�д�벢��ȡһ���������ֽڡ�
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

// �ֲ��49ҳ
/* �Ĵ���               8         7          6         5         4         3         2        1 */
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
        tx_cfg[i][0] = 0xFE ;   //GPIO����������·�ضϣ�bit8~bit4�� | ��׼�����ϵ�״̬��bit3�� |
SWTEN�����߼�1�������ʱ���� | ADCģʽѡ��Ϊ0 tx_cfg[i][1] = 0x00 ;   //��ʹ��Ƿѹ�ȽϹ���
        tx_cfg[i][2] = 0x00 ;   //��ʹ�ù�ѹ�ȽϹ���
        tx_cfg[i][3] = 0x00 ;
        tx_cfg[i][4] = 0x00 ;   //��ʹ�õ�طŵ繦��
        tx_cfg[i][5] = 0x00 ;   //�ŵ糬ʱʱ��
    }
}
*/
