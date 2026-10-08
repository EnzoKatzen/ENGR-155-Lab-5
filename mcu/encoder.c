// encoder.c

#include "encoder.h"
#include "STM32L432KC_EXTI.h"

// Shared variables
static volatile int	encoder_count;
static volatile uint32_t last_edge_us;
static volatile uint32_t last_edge_interval_us;

//Adds or removes a step from count if going CW or CCW
static void recordEdge(int16_t step) {
	uint32_t now_us = TIM2->CNT;
	encoder_count += step;
	last_edge_interval_us = now_us - last_edge_us;
	last_edge_us = now_us;
}

void encoderInit(void) {
	gpioEnable(GPIO_PORT_A);
	pinMode(ENCODER_A_PIN, GPIO_INPUT);
	pinMode(ENCODER_B_PIN, GPIO_INPUT);

	//Top of page 56 of datasheet: "To sustain a voltage higher than 4 V the internal pull-up/pull-down resistors must be disabled."
	uint32_t a_pupd_mask = 0b11u << (2 * gpioPinOffset(ENCODER_A_PIN));
	uint32_t b_pupd_mask = 0b11u << (2 * gpioPinOffset(ENCODER_B_PIN));
	GPIOA->PUPDR &= ~a_pupd_mask;
	GPIOA->PUPDR &= ~b_pupd_mask;

	encoder_count = 0;
	last_edge_us = TIM2->CNT;
	last_edge_interval_us = 0;

	extiEnableBothEdges(ENCODER_A_PIN);
	extiEnableBothEdges(ENCODER_B_PIN);

	NVIC_EnableIRQ(EXTI9_5_IRQn);	//PA9 is on pin line 9. Acronym from RM pg 322
	NVIC_EnableIRQ(EXTI15_10_IRQn);	//PA10 is on pin line 10. Acronym from RM pg 322
}

//Encoder A input (EXTI lines 5-9)
void EXTI9_5_IRQHandler(void) {
	uint32_t a_line_mask = 1u << gpioPinOffset(ENCODER_A_PIN); //mask for that specific pin
	if (EXTI->PR1 & a_line_mask) {
		// Clear first (write 1 to reset) so an edge during handling re-triggers
		EXTI->PR1 = a_line_mask;

		if (digitalRead(ENCODER_A_PIN) != digitalRead(ENCODER_B_PIN)) {
			recordEdge(1);	//If they don't equal, then A moved away from B, so that means A leads, so it is going forward
		} else {
			recordEdge(-1);	//If they do equal, then A caught up to B, so that means B leads, so it is going backwards
		}
	}
}

//Encoder B input (EXTI lines 10-15)
void EXTI15_10_IRQHandler(void) {
	uint32_t b_line_mask = 1u << gpioPinOffset(ENCODER_B_PIN);
	if (EXTI->PR1 & b_line_mask) {
		EXTI->PR1 = b_line_mask;

		if (digitalRead(ENCODER_A_PIN) == digitalRead(ENCODER_B_PIN)) {
			recordEdge(1);	//If they do equal, then B caught up to A, so that means A leads, so it is going forward
		} else {
			recordEdge(-1);	//If they don't equal, then B moved away from A, so that means B leads, so it is going backwards
		}
	}
}


//120 pulses per rev = 120 pulses/rev per channel x 4 edges (rising and falling edges of A and B)
static float countsToRevPerSec(int counts, uint32_t elapsed_us) {
	if (elapsed_us == 0) {
		return 0.0f;	//Stop divide by 0
	}
	return ((float) counts / 480.0f) * (1000000.0f / (float) elapsed_us);
}

//Returns signed velocity in rev/s
//Velocity = (counts since last call) / (time between the edge seen by the old call and the edge seen now)
float encoderGetVelocity(void) {
	//Remembered between calls
	static int prev_count = 0;
	static uint32_t prev_edge_us = 0;
	static int stopped = 1;
	static float velocity_rps = 0.0f;

	//Copy the values. Pause interrupts so nothing gets overwritten while we are copying
	__disable_irq();
	int count = encoder_count;
	uint32_t edge_us = last_edge_us;
	uint32_t interval_us = last_edge_interval_us;
	uint32_t now_us = TIM2->CNT;
	__enable_irq();

	int new_counts = count - prev_count;

	if (now_us - edge_us >= 1000000u) {	//If there have been no edges for 1s the motor is stopped
		//120 PPR & 1s means min RPM is 60/120 = 0.5. Can make slower, but I think that is fine.
		velocity_rps = 0.0f;
		stopped = 1;
	} else if (new_counts != 0 && stopped) { 
		//If we are starting from being stopped, the time from the previous edge is useless, so use the latest edge interval
		velocity_rps = countsToRevPerSec((new_counts > 0) ? 1 : -1, interval_us);
		stopped = 0;
	} else if (new_counts != 0) {
		velocity_rps = countsToRevPerSec(new_counts, edge_us - prev_edge_us);
	}else{
            //if nothing has happened, but it hasn't been a second yet, make test pulse to make it seem like there was a change
            float temp = countsToRevPerSec(1, now_us-edge_us);
            if (velocity_rps > temp) { //then, set velocity to it if the temp value has a lower magnitude than the old velocity, matching sign
                velocity_rps = temp;
            } else if (velocity_rps < -temp) {
                velocity_rps = -temp;
            }
        }

	prev_count = count;
	prev_edge_us = edge_us;
	return velocity_rps;
}