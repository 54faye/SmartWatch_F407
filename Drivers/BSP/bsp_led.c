#include "bsp_led.h"

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = LED_RED_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_RED_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = LED_BLUE_PIN;
    GPIO_Init(LED_BLUE_GPIO, &GPIO_InitStructure);

    LED_RED_ON;
    LED_BLUE_OFF;
}

/* 兼容原项目函数接口 (LED1 = 红色LED, PC5) */
void LED1_ON(void)  { LED_RED_ON; }
void LED1_OFF(void) { LED_RED_OFF; }
void LED1_Turn(void) { LED_RED_TOGGLE; }
