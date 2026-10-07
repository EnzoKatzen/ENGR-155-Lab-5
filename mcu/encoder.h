// encoder.h
// Header for Encoder functions

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stm32l432xx.h>
#include "STM32L432KC_GPIO.h"

// Encoder signal pins both need to be FT
#define ENCODER_A_PIN PA9
#define ENCODER_B_PIN PA10

void encoderInit(void);
float encoderGetVelocity(void);

#endif // ENCODER_H