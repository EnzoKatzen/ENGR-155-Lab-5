// STM32L432KC_EXTI.h
// Header for EXTI functions

#ifndef STM32L4_EXTI_H
#define STM32L4_EXTI_H

#include <stdint.h>
#include <stm32l432xx.h>

void extiEnableBothEdges(int gpio_pin);

#endif