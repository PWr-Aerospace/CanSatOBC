#pragma once
#include <stdint.h>

// M is 0 to 63
// N is 0 to 511
// Q is 0 to 127
// P is 0 to 127
void clock_setup_HSE(uint8_t M, uint32_t N, uint8_t Q, uint8_t P);
void sleep_ms(uint32_t ms);

extern "C" void SysTick_Handler(void);
extern "C" void HAL_Delay(uint32_t);
extern "C" uint32_t HAL_GetTick();
