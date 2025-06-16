#include "uart.hpp"
#include "config.h"

#include <stdint.h>
#include <string.h>

#include "etl/queue.h"
#include "etl/string.h"

#include "stm32h533xx.h"

uint8_t uart4_busy = 0;

constexpr uint32_t max_messages_no = 16;
//constexpr uint32_t UART4_RX_BUFFER_SIZE = 1023;
uint8_t uart4_receive_buffer[UART4_RX_BUFFER_SIZE];

etl::queue<etl::string<UART4_RX_BUFFER_SIZE + 1>, max_messages_no> queue;

void
print(const char* str)
{
  if (!str[0])
    return;
  while (uart4_busy)
    ;
  uart4_busy = 1;
  uint32_t bytes_amount = strlen(str);
  GPDMA1_Channel0->CBR1 = bytes_amount;
  GPDMA1_Channel0->CSAR = (uint32_t) str;
  GPDMA1_Channel0->CDAR = (uint32_t) &UART4->TDR;
  UART4->ICR |= USART_ICR_TCCF;
  GPDMA1_Channel0->CCR = DMA_CCR_TCIE | DMA_CCR_EN;
}

void
uart4_receive_to_idle()
{
  UART4->ICR |= USART_ICR_IDLECF;
  GPDMA1_Channel1->CBR1 = UART4_RX_BUFFER_SIZE;
  GPDMA1_Channel1->CDAR = (uint32_t) uart4_receive_buffer;
  GPDMA1_Channel1->CSAR = (uint32_t) &UART4->RDR;
  GPDMA1_Channel1->CCR = DMA_CCR_EN;
}

void
uart4_setup()
{
  // DMA UART4 TX
  RCC->AHB1ENR |= RCC_AHB1ENR_GPDMA1EN;
  GPDMA1_Channel0->CTR1 |= DMA_CTR1_SINC;
  GPDMA1_Channel0->CTR2 = 28;
  //
  // DMA UART4 RX
  GPDMA1_Channel1->CTR1 |= DMA_CTR1_DINC;
  GPDMA1_Channel1->CTR2 = 27;

  NVIC_SetPriority(GPDMA1_Channel0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
  NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);

  RCC->APB1LENR |= RCC_APB1LENR_UART4EN;
  RCC->CCIPR1 &= ~RCC_CCIPR1_UART4SEL_Msk; // Set clock source as main PLL
  UART4->CR1 &= ~USART_CR1_M;
  UART4->BRR = (desiredMhz * 1000000 / 57600);
  UART4->CR2 &= ~USART_CR2_STOP;
  UART4->CR2 |= USART_CR2_SWAP;
  UART4->CR3 |= (USART_CR3_DMAT | USART_CR3_DMAR);
  UART4->CR1 |= USART_CR1_IDLEIE;
  UART4->CR1 |= USART_CR1_UE;
  UART4->CR1 |= (USART_CR1_TE | USART_CR1_RE);
  //   Clear initial IDLE flag
  (void) UART4->ISR;
  (void) UART4->RDR;
  UART4->ICR |= USART_ICR_IDLECF;
  NVIC_SetPriority(UART4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 2));
  NVIC_EnableIRQ(UART4_IRQn);

  uart4_receive_to_idle();
}

void
__uart4_received_message(uint8_t* cstr)
{
  if (queue.size() >= max_messages_no)
    return;
  queue.push((const char*) cstr);
}

bool
uart4_data_received(etl::string<UART4_RX_BUFFER_SIZE + 1>& str)
{
  if (queue.empty())
    return false;
  //	str = std::move(queue.front());
  queue.pop_into(str);
  return true;
  //	return queue.front();
}

void
uart6_setup()
{
  RCC->APB1LENR |= RCC_APB1LENR_USART6EN;
  RCC->CCIPR1 &= ~RCC_CCIPR1_USART6SEL; // Set clock source as main PLL
  USART6->CR1 &= ~USART_CR1_M;
  USART6->BRR = (desiredMhz * 1000000 / 57600);
  USART6->CR2 &= ~USART_CR2_STOP;
  USART6->CR2 |= USART_CR2_SWAP;
  USART6->CR1 |= USART_CR1_TE;
}
