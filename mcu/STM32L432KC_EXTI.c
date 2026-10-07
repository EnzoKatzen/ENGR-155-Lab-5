// STM32L432KC_EXTI.c
// Source code for EXTI functions

#include "STM32L432KC_EXTI.h"
#include "STM32L432KC_GPIO.h"


//I put a fair bit of time into making this ultra flexible as I have a feeling having a good interrupt function will be very helpful
void extiEnableBothEdges(int gpio_pin) {
	int line = gpioPinOffset(gpio_pin);
	int exticr_index = line / 4;	    //4 pin lines in each EXTICR register
	int exticr_shift = 4 * (line % 4);	//4 bits of registers per pin line (but only 3 are actually set)
	uint32_t line_mask = 1u << line;	//make a mask for the registers in EXTI
	uint32_t exticr_field_mask = 0b111u << exticr_shift; //make a mask for the 3 bits in the SYSCFG->EXTICR register for each pin
	uint32_t exticr_port_code = (uint32_t) gpioPinToPort(gpio_pin) << exticr_shift; //Get which port, we are using, and enbale for that port 

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //turn on clk for interrupts

	SYSCFG->EXTICR[exticr_index] &= ~exticr_field_mask;	//Use mask to clear existing data
	SYSCFG->EXTICR[exticr_index] |= exticr_port_code;	//Set the interrupt to point at right port

	EXTI->RTSR1 |= line_mask;	//Turn on for rising edge
	EXTI->FTSR1 |= line_mask;   //Turn on for falling edge
	EXTI->PR1 = line_mask;		//Reset any flags
	EXTI->IMR1 |= line_mask;	//unmask so we can see what is happening
}

//Looking at an example makes this more sensible: extiEnableBothEdges(PA9)
//line				= 9				PA9 is pin 9 of port A, so enable it using EXTI line 9
//exticr_index			= 9 / 4 = 2			Pin line 9 is set in EXTICR3, but as arrays start at 0, that is EXTICR[2]
//exticr_shift			= 4 * (9 % 4) = 4               Pin line 9 is bits 6:4 of the right EXTICR register
//line_mask                     = 1 << 9 = 0x200                Bit 9 in IMR1, RTSR1, FTSR1, and PR1
//exticr_field_mask             = 0b111 << 4			mask on bits 6:4
//exticr_port_code              = 0 << 4 = 0			Port A code is 000
//
//Result:
//EXTICR[2] bits 6:4 = 000                                      Line 9 now comes from port A (GPIO pins A)
//RTSR1 bit 9 = 1                                               Trigger on rising edge
//FTSR1 bit 9 = 1						Trigger on falling edge
//PR1 bit 9 cleared                                             Reset any interrupts
//IMR1 bit 9 = 1						Unmasked so we can see them
