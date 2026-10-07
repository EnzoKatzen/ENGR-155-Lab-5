// STM32F401RE_TIM.c
// TIM functions

#include "STM32L432KC_TIM.h"
#include "STM32L432KC_RCC.h"
//Uh this breaks at 80 MHz as prescaler maxes out at 65535, but it needs to be 80000 using this code...
void initTIM_ms(TIM_TypeDef * TIMx){
	// Set prescaler to give 1 ms time base
	uint32_t psc_div = (uint32_t) ((SystemCoreClock/1e3));
	// Set prescaler division factor
	TIMx->PSC = (psc_div - 1);
	// Generate an update event to update prescaler value
	TIMx->EGR |= 1;
	// Enable counter
	TIMx->CR1 |= 1; // Set CEN = 1
}

void initTIM_us(TIM_TypeDef * TIMx){
	// Set prescaler to give 1 us time base for extra precision
	uint32_t psc_div = (uint32_t) ((SystemCoreClock/1e6));
	// Set prescaler division factor
	TIMx->PSC = (psc_div - 1);
	// Generate an update event to update prescaler value
	TIMx->EGR |= 1;
	// Enable counter
	TIMx->CR1 |= 1; // Set CEN = 1
}

void delay_millis(TIM_TypeDef * TIMx, uint32_t ms){
    if(ms == 0) return; // change in following line necessitates
	TIMx->ARR = ms-1;   // Set timer max count (-1 so it is exact vs 1 ms longer)
	TIMx->EGR |= 1;		// Force update
	TIMx->SR &= ~(0x1); // Clear UIF
	TIMx->CNT = 0;		// Reset count

	while(!(TIMx->SR & 1)); // Wait for UIF to go high
}