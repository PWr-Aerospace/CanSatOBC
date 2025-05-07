#include "stdint.h"
#include "stm32h533xx.h"

extern uint8_t uart4_busy;

void
GPDMA1_CH0_IRQHandler(void)
{
  if (GPDMA1_Channel0->CSR & DMA_CSR_TCF)
  {
    GPDMA1_Channel0->CFCR |= DMA_CFCR_TCF;
    uart4_busy = 0;
  }
}

extern uint8_t gotMessage;
extern uint8_t uart4_receive_buffer[1024];

void
UART4_IRQHandler(void)
{
  if (UART4->ISR & USART_ISR_IDLE)
  {
    UART4->ICR |= USART_ICR_IDLECF;

    if (GPDMA1_Channel1->CCR & DMA_CCR_EN)
    {
      *((uint8_t*) GPDMA1_Channel1->CDAR) = 0;
      GPDMA1_Channel1->CCR |= DMA_CCR_SUSP;
      while (!(GPDMA1_Channel1->CSR & DMA_CSR_SUSPF))
        ;
      GPDMA1_Channel1->CCR |= DMA_CCR_RESET;
      gotMessage = 1;
    }
  }
}
