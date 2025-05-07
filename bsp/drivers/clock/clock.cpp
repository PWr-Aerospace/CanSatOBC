#include "clock.hpp"

#include "stm32h533xx.h"

#include <stdint.h>

/*
 * f_out = f_in *N/M /Q
 *
 */
void
clock_setup_HSE(uint8_t M, uint32_t N, uint8_t Q, uint8_t P)
{
  // Enable HSE
  RCC->CR |= RCC_CR_HSEON_Msk;

  while (!(RCC->CR & RCC_CR_HSERDY))
    ;

  // Disable PLL 1
  RCC->CR &= ~RCC_CR_PLL1ON;

  // Set source for PLL 1
  RCC->PLL1CFGR |= (M << RCC_PLL1CFGR_PLL1M_Pos); // Set dividers for PLL 1
  //Enable outputs Q and P
  RCC->PLL1CFGR |= RCC_PLL1CFGR_PLL1QEN | RCC_PLL1CFGR_PLL1PEN;

  RCC->PLL1DIVR =
      (((Q - 1) << RCC_PLL1DIVR_PLL1Q_Pos) | ((P - 1) << RCC_PLL1DIVR_PLL1P_Pos) | (N - 1));
  // Set HSE as source for PLL 1
  RCC->PLL1CFGR |= 3 << RCC_PLL1CFGR_PLL1SRC_Pos;
  // Enable PLL 1
  RCC->CR |= RCC_CR_PLL1ON;

  // Wait for PLL to stabilize
  while (!(RCC->CR & RCC_CR_PLL1RDY_Msk))
    ;

  // Set PLL 1 as system clock
  RCC->CFGR1 |= (3 << RCC_CFGR1_SW_Pos);
  while (!(RCC->CFGR1 & (RCC_CFGR1_SWS_0 | RCC_CFGR1_SWS_0)))
    ;
  const uint32_t ticks_per_second = 16000000 / M * N / P;
  if (SysTick_Config(ticks_per_second / 1000))
  {
    while (true)
      ;
  }
}

static uint32_t ticks = 0;

void
SysTick_Handler(void)
{
  ticks++;
}

uint32_t
SysTick_get_tick_count()
{
  return ticks;
}

void
sleep_ms(uint32_t ms)
{
  uint32_t start = ticks;
  while (ticks < (start + ms))
    ;
}
