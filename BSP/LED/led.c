#include "./BSP/LED/led.h"

/**
 * @brief       初始化led外设
 * @param       无
 * @return      无
 */
void led_init(void) {
   GPIO_InitTypeDef led_gpio_init;

   __HAL_RCC_GPIOF_CLK_ENABLE();  // PF口时钟使能

   led_gpio_init.Pin = LED0_GPIO_PIN;           // led0引脚
   led_gpio_init.Mode = GPIO_MODE_OUTPUT_PP;    // 推挽输出
   led_gpio_init.Pull = GPIO_PULLUP;            // 上拉
   led_gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  // 高速
   HAL_GPIO_Init(LED0_GPIO_PORT, &led_gpio_init);

   led_gpio_init.Pin = LED1_GPIO_PIN;           // led1的引脚
   led_gpio_init.Mode = GPIO_MODE_OUTPUT_PP;    // 推挽输出
   led_gpio_init.Pull = GPIO_PULLUP;            // 上拉
   led_gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  // 高速
   HAL_GPIO_Init(LED1_GPIO_PORT, &led_gpio_init);

   LED0(1);  // 关闭led0
   LED1(1);  // 关闭lde1
}
