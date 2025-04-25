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
   HAL_Init();                         /* ��ʼ��HAL�� */
   sys_stm32_clock_init(336, 8, 2, 7); /* ����ʱ��,168Mhz */
   delay_init(168);                    /* ��ʱ��ʼ�� */
   usart_init(115200);                 /* ���ڳ�ʼ��Ϊ115200 */
   led_init();                         /* ��ʼ��LED */
   relay_init();                       /* ��ʼ���̵��� */
   LTC6804_init();                     /* ��ʼ��6804 */

   while (1) {
      delay_ms(1000);
      LED0(0);

      delay_ms(1000);
      LED0(1);
      CS_LTC6804(0);  // ����Ƭѡ

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
