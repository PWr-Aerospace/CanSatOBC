#pragma once

#include "etl/callback_service.h"
#include "etl/function.h"
#include "stm32h533xx.h"

constexpr auto VECTOR_ID_OFFSET = SPI1_IRQn;
enum class VectorId
{
  SPI_1 = SPI1_IRQn,
  SPI_2 = SPI2_IRQn,
  SPI_3 = SPI3_IRQn,
  USART_1 = USART1_IRQn,
  USART_2 = USART2_IRQn,
  USART_3 = USART3_IRQn,
  UART_4 = UART4_IRQn,
  UART_5 = UART5_IRQn,
  __END,
  __OFFSET = SPI_1,
  __RANGE = __END - __OFFSET
};

constexpr auto VECTOR_ID_RANGE =
    static_cast<uint32_t>(VectorId::__END) - static_cast<uint32_t>(VectorId::__OFFSET);

typedef etl::callback_service<VECTOR_ID_RANGE, VECTOR_ID_OFFSET> InterruptVectors;

// Ensure that the callback service is initialised before use.
InterruptVectors& GetInterruptVectorsInstance();

#ifdef __cplusplus
extern "C" {
#endif

void GPDMA1_CH0_IRQHandler(void);
void UART4_IRQHandler(void);
void HardFault_Handler(void);

#ifdef __cplusplus
}
#endif
