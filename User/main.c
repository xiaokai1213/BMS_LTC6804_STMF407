#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

#include "./BSP/LED/led.h"
#include "./BSP/RELAY/relay.h"

#include "./BSP/LTC6804_LTC6820_SPI1/ltc6804_1.h"

uint16_t cell_codes[1][12];
uint16_t aux_codes[1][6];
unsigned char TOTAL_IC = 1;

int main(void) {
   HAL_Init();                         /* 初始化HAL库 */
   sys_stm32_clock_init(336, 8, 2, 7); /* 设置时钟,168Mhz */
   delay_init(168);                    /* 延时初始化 */
   usart_init(115200);                 /* 串口初始化为115200 */
   led_init();                         /* 初始化LED */
   relay_init();                       /* 初始化继电器 */
   LTC6804_init();                     /* 初始化6804 */

   while (1) {
      delay_ms(1000);
      LED0(0);

      delay_ms(1000);
      LED0(1);
      CS_LTC6804(0);  // 拉低片选

      wakeup_sleep();
      LTC6804_adcv();
      delay_ms(50);
      LTC6804_rdcv(0, TOTAL_IC, cell_codes);
      printf("V1:%fv \r\n", (float)cell_codes[0][0] / 10000);
      printf("V2:%fv \r\n", (float)cell_codes[0][1] / 10000);
      printf("V3:%fv \r\n", (float)cell_codes[0][2] / 10000);
      printf("V4:%fv \r\n", (float)cell_codes[0][6] / 10000);
      printf("V5:%fv \r\n", (float)cell_codes[0][7] / 10000);
      printf("\r\n");
   }
}
