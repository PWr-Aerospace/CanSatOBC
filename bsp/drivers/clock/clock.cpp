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

  // Setup PLL 2 for SD card
  uint8_t pll2_m = 16;
  uint32_t pll2_n = 192;
  uint8_t pll2_r = 4;
  // set as 100 MHz
  RCC->CR &= ~RCC_CR_PLL2ON;

  // Set source for PLL 2
  RCC->PLL2CFGR |= (pll2_m << RCC_PLL2CFGR_PLL2M_Pos); // Set dividers for PLL 1
  //Enable outputs $
  RCC->PLL2CFGR |= RCC_PLL2CFGR_PLL2REN;

  RCC->PLL2DIVR |= ((pll2_r - 1) << RCC_PLL2DIVR_PLL2R_Pos | ((pll2_n - 1) & RCC_PLL2DIVR_PLL2N));
  // Set HSE as source for PLL 1
  RCC->PLL2CFGR |= 3 << RCC_PLL2CFGR_PLL2SRC_Pos;
  // Enable PLL 2
  RCC->CR |= RCC_CR_PLL2ON;

  // Wait for PLL to stabilize
  while (!(RCC->CR & RCC_CR_PLL2RDY_Msk))
    ;
  // Makes SDMMC use PLL 2
  RCC->CCIPR4 |= (1 << RCC_CCIPR4_SDMMC1SEL_Pos);

  RCC->AHB1ENR |= RCC_AHB1ENR_BKPRAMEN;
}
static uint32_t ticks = 0;

void
sleep_ms(uint32_t ms)
{
  uint32_t start = ticks;
  while (ticks < (start + ms))
    ;
}

extern "C" {

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

uint32_t
HAL_GetTick()
{
  return ticks;
}

void
HAL_Delay(uint32_t t)
{
  sleep_ms(t);
}
}
