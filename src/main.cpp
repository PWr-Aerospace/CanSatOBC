#include "drivers/clock/clock.hpp"
#include "drivers/gpio/gpio.hpp"

#include "stm32h533xx.h"
#include "system/system.h"
#include <stdint.h>

int
main()
{
  const uint32_t desiredMhz = 125;
  clock_setup_HSE(16, desiredMhz, 1, 1);

  Gpio dbgLed('C', 13, Mode::Out);
  Gpio mux('A', 10, Mode::Out);
  Gpio xbeeRst('A', 4, Mode::Out);
  Gpio uart4_tx('D', 12, Mode::AF, 8);
  Gpio uart4_rx('D', 11, Mode::AF, 8);

  Gpio usart6_tx('C', 6, Mode::AF, 7);
  Gpio usart6_rx('C', 7, Mode::AF, 7);

  xbeeRst.set();
  mux.set();

  RCC->APB1LENR |= RCC_APB1LENR_UART4EN;
  RCC->CCIPR1 &= ~RCC_CCIPR1_UART4SEL_Msk; // Set clock source as main PLL
  UART4->CR1 &= ~USART_CR1_M;
  UART4->BRR = (desiredMhz * 1000000 / 57600);
  UART4->CR2 &= ~USART_CR2_STOP;
  UART4->CR2 |= USART_CR2_SWAP;
  UART4->CR1 |= USART_CR1_UE;
  UART4->CR1 |= USART_CR1_TE;

  RCC->APB1LENR |= RCC_APB1LENR_USART6EN;
  RCC->CCIPR1 &= ~RCC_CCIPR1_USART6SEL; // Set clock source as main PLL
  USART6->CR1 &= ~USART_CR1_M;
  USART6->BRR = (desiredMhz * 1000000 / 57600);
  USART6->CR2 &= ~USART_CR2_STOP;
  USART6->CR1 |= USART_CR1_UE;
  USART6->CR1 |= USART_CR1_TE;

  while (1)
  {
    sleep_ms(1000);
    dbgLed.toggle();
    UART4->TDR = 'A';
    while (!(UART4->ISR & USART_ISR_TC))
      ;
    USART6->TDR = 'A';
    while (!(USART6->ISR & USART_ISR_TC))
      ;
  }
}
