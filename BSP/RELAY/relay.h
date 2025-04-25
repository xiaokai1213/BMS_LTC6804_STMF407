#ifndef _RELAY_H
#define _RELAY_H

#include "./SYSTEM/sys/sys.h"

/* 引脚定义 */

#define RELAY_AIR_cat_PORT GPIOB
#define RELAY_AIR_cat_PIN  GPIO_PIN_0

#define RELAY_PRE_PORT     GPIOB
#define RELAY_PRE_PIN      GPIO_PIN_1

#define RELAY_AIR_neg_PORT GPIOA
#define RELAY_AIR_neg_PIN  GPIO_PIN_15

/******************************************************************************************/
/* 继电器端口定义 */
/*主正继电器*/
#define RELAY_AIR_CAT(X)                                                            \
   do {                                                                             \
      X ? HAL_GPIO_WritePin(RELAY_AIR_cat_PORT, RELAY_AIR_cat_PIN, GPIO_PIN_SET)    \
        : HAL_GPIO_WritePin(RELAY_AIR_cat_PORT, RELAY_AIR_cat_PIN, GPIO_PIN_RESET); \
   } while (0)

/*预充继电器*/
#define RELAY_PRE(X)                                                        \
   do {                                                                     \
      X ? HAL_GPIO_WritePin(RELAY_PRE_PORT, RELAY_PRE_PIN, GPIO_PIN_SET)    \
        : HAL_GPIO_WritePin(RELAY_PRE_PORT, RELAY_PRE_PIN, GPIO_PIN_RESET); \
   } while (0)

/*主负继电器*/
#define RELAY_AIR_NEG(X)                                                            \
   do {                                                                             \
      X ? HAL_GPIO_WritePin(RELAY_AIR_neg_PORT, RELAY_AIR_neg_PIN, GPIO_PIN_SET)    \
        : HAL_GPIO_WritePin(RELAY_AIR_neg_PORT, RELAY_AIR_neg_PIN, GPIO_PIN_RESET); \
   } while (0)

/******************************************************************************************/
/* 外部接口函数*/

void relay_init(void);

#endif  // _RELAY_H
