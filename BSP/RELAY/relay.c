#include "./BSP/RELAY/relay.h"
/**
 * @brief 继电器控制初始化
 * @param 无
 * @return 无
 */

void relay_init(void) {
   GPIO_InitTypeDef relay_gpio_init;

   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();

   relay_gpio_init.Pin = RELAY_AIR_cat_PIN;
   relay_gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
   relay_gpio_init.Pull = GPIO_PULLDOWN;
   relay_gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(RELAY_AIR_cat_PORT, &relay_gpio_init);

   relay_gpio_init.Pin = RELAY_PRE_PIN;
   HAL_GPIO_Init(RELAY_PRE_PORT, &relay_gpio_init);

   relay_gpio_init.Pin = RELAY_AIR_neg_PIN;
   HAL_GPIO_Init(RELAY_AIR_neg_PORT, &relay_gpio_init);

   RELAY_AIR_CAT(0);
   RELAY_PRE(0);
   RELAY_AIR_NEG(0);
}
