// main.c

#include <math.h>
#include "main.h"

//Prints direction and speed
void printMotorSpeed(float velocity_rps) {
	const char * direction = "stopped"; //use a pointer to an array of chars as a string
	if (velocity_rps > 0.0f) {
		direction = "CW";
	} else if (velocity_rps < 0.0f) {
		direction = "CCW";
	}
        float absVelocity = fabsf(velocity_rps);
	printf("%s %f rev/s\n", direction, absVelocity);
}

//function used by printf
int _write(int file, char * ptr, int len) {
	(void) file;
	for (int i = 0; i < len; i++) {
		ITM_SendChar(ptr[i]);
	}
	return len;
}

void enableClocks(void){
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;	//for being a 1us clk
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;	//for being a 1ms clk for delays
}

int main(void) {
	configureHSI16();	// first start the clock. Needs to be 16 MHz as given code breaks at 80 MHz

    enableClocks();
	initTIM_us(TIM2); //microsecond precision for checking encoder
	initTIM_ms(TIM6); //millisecond precision for print statements

	encoderInit();
	__enable_irq();

	while (1) {
		delay_millis(TIM6, 250);	//4 Hz update
		printMotorSpeed(encoderGetVelocity());
	}
}