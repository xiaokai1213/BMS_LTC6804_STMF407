#ifndef _LED_H
#define _LED_H

#include "./SYSTEM/sys/sys.h"

/*led1已经损坏，PF10口已无法使用*/

/* 引脚定义 */

#define LED0_GPIO_PORT GPIOF
#define LED0_GPIO_PIN GPIO_PIN_9

#define LED1_GPIO_PORT GPIOF
#define LED1_GPIO_PIN GPIO_PIN_10

/******************************************************************************************/
/* LED端口定义 */
#define LED0(x)                                                             \
   do {                                                                     \
      x ? HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_SET)    \
        : HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_RESET); \
   } while (0)
/* LED0 = RED */

#define LED1(x)                                                             \
   do {                                                                     \
      x ? HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_SET)    \
        : HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_RESET); \
   } while (0)
/* LED1 = GREEN */

/* LED取反定义 */
#define LED0_TOGGLE()                                    \
   do {                                                  \
      HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_GPIO_PIN); \
   } while (0)
/* LED0 = !LED0 */

#define LED1_TOGGLE()                                    \
   do {                                                  \
      HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_GPIO_PIN); \
   } while (0)
/* LED1 = !LED1 */

/******************************************************************************************/
/* 外部接口函数*/

void led_init(void);

#endif  // _LED_H
