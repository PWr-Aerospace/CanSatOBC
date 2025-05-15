#include "drivers/clock/clock.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/i2c/i2c.hpp"

#include "etl/string.h"
#include "interrupts.h"
#include "stm32h533xx.h"
#include "system/system.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SDIO_MAX_IO_NUMBER 7U

#include "stm32h5xx_hal_sd.h"
#include "stm32h5xx_hal_sdio.h"

uint8_t uart4_busy = 0;

void
sd_test()
{
  //__HAL_RCC_SDMMC1_CLK_ENABLE
  RCC->AHB4ENR |= RCC_AHB4ENR_SDMMC1EN;
  RCC->CCIPR4 |= (1 << RCC_CCIPR4_SDMMC1SEL_Pos);

  //	MMC_HandleTypeDef hmmc1;
  SDIO_HandleTypeDef sdio1;

  sdio1.Instance = SDMMC1;
  sdio1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  sdio1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  sdio1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  sdio1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  sdio1.Init.ClockDiv = 3;
  HAL_StatusTypeDef result = HAL_SDIO_Init(&sdio1);
  if (result != HAL_OK)
  {
    while (1)
      ;
  }
  //  SD_HandleTypeDef hsd1;
  //
  //  hsd1.Instance = SDMMC1;
  //  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  //  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  //  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B; // Start with 1-bit
  //  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  //  hsd1.Init.ClockDiv = 4;
  //  if (HAL_SD_Init(&hsd1) != HAL_OK)
  //  {
  //    while (1)
  //      ;
  //  }
}
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
constexpr const uint32_t UART4_RX_BUFFER_SIZE = 1024;
uint8_t uart4_receive_buffer[UART4_RX_BUFFER_SIZE];

void
uart4_receive_to_idle()
{
  UART4->ICR |= USART_ICR_IDLECF;
  GPDMA1_Channel1->CBR1 = UART4_RX_BUFFER_SIZE;
  GPDMA1_Channel1->CDAR = (uint32_t) uart4_receive_buffer;
  GPDMA1_Channel1->CSAR = (uint32_t) &UART4->RDR;
  GPDMA1_Channel1->CCR = DMA_CCR_EN;
}
uint8_t gotMessage = false;

int
main()
{
  SystemCoreClockUpdate();
  etl::string<12> str;
  str.clear();

  const uint32_t desiredMhz = 125;
  clock_setup_HSE(16, desiredMhz, 1, 1);

  Gpio dbgLed('C', 13, Mode::Out);
  Gpio mux('A', 10, Mode::Out);
  Gpio xbeeRst('A', 4, Mode::Out);
  Gpio uart4_tx('D', 12, Mode::AF, Type::OD, 8);
  Gpio uart4_rx('D', 11, Mode::AF, Type::OD, 8);

  Gpio usart6_tx('C', 6, Mode::AF, Type::OD, 7);
  Gpio usart6_rx('C', 7, Mode::AF, Type::OD, 7);

  Gpio scl('B', 10, Mode::AF, Type::OD, 4);
  Gpio sda('B', 12, Mode::AF, Type::OD, 4);

  Gpio sd_d0('C', 8, Mode::AF, Type::OD, Pull::Up, Speed::VeryHigh,12);
  Gpio sd_d1('C', 9, Mode::AF, Type::OD, Pull::Up, Speed::VeryHigh,12);
  Gpio sd_d2('C', 10, Mode::AF, Type::OD, Pull::Up, Speed::VeryHigh,12);
  Gpio sd_d3('C', 11, Mode::AF, Type::OD, Pull::Up, Speed::VeryHigh,12);
  Gpio sd_clk('C', 12, Mode::AF, Type::OD, Pull::None, Speed::VeryHigh,12);
  Gpio sd_cmd('D', 2, Mode::AF, Type::OD, Pull::None, Speed::VeryHigh,12);
  (void) sd_d0;
  (void) sd_d1;
  (void) sd_d2;
  (void) sd_d3;
  (void) sd_clk;
  (void) sd_cmd;

  xbeeRst.set();
  mux.set();

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

  RCC->APB1LENR |= RCC_APB1LENR_USART6EN;
  RCC->CCIPR1 &= ~RCC_CCIPR1_USART6SEL; // Set clock source as main PLL
  USART6->CR1 &= ~USART_CR1_M;
  USART6->BRR = (desiredMhz * 1000000 / 57600);
  USART6->CR2 &= ~USART_CR2_STOP;
  USART6->CR2 |= USART_CR2_SWAP;
  USART6->CR1 |= USART_CR1_TE;
  I2C2_setup();
  uint8_t bq_config[1] = {1 << 7 | 3 << 4 | 1 << 2};
  I2C2_Master_Write(0x6B, 0x26, bq_config, 1);
  uint8_t data[16] = {};
  //  uint16_t raw_reading = 0;
  float vsys = 0;
  float vbat = 0;

  while (1)
  {
    sd_test();
    uart4_receive_to_idle();
    sleep_ms(1000);
    dbgLed.toggle();

    I2C2_Master_Read(0x6B, 0x30, data, 4);
    //    raw_reading = (data[1] << 8 | data[0]) >> 1;
    vbat = ((data[1] << 8 | data[0]) >> 1) * 1.99;
    vsys = ((data[3] << 8 | data[2]) >> 1) * 1.99;
    static char message[128]; //= "Hello world from CanSat!!!\r\n";
    snprintf(message,
             128,
             "System: %f mV\r\nBattery: %f mV\r\n",
             vsys,
             vbat); // @suppress("Float formatting support")
    print((char*) message);
    if (gotMessage)
    {
      print((const char*) uart4_receive_buffer);
      uart4_receive_to_idle();
      gotMessage = 0;
    }
  }
}
