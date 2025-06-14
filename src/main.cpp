#include "drivers/clock/clock.hpp"
#include "drivers/gpio/gpio.hpp"

#include "etl/string.h"
#include "interrupts.h"
#include "stm32h533xx.h"
#include "system/system.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

uint8_t uart4_busy = 0;

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

// Function to read data from an I2C slave in Master mode
// slave_addr: 7-bit slave address
// reg_addr: Register address on the slave to read from
// data: Pointer to data buffer
// len: Number of bytes to read
uint8_t I2C1_Master_Read(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    uint16_t i;
    slave_addr = slave_addr << 1;

    // Check for BUSY flag
    while ((I2C2->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);

    // Send register address (write phase)
    I2C2->CR2 = (slave_addr << I2C_CR2_SADD_Pos) | (1 << I2C_CR2_NBYTES_Pos) | I2C_CR2_START | (0 << I2C_CR2_RD_WRN_Pos);

    while ((I2C2->ISR & I2C_ISR_TXIS) == 0 && (I2C2->ISR & I2C_ISR_NACKF) == 0);
    if (I2C2->ISR & I2C_ISR_NACKF) {
        I2C2->ICR |= I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        return 1;
    }
    I2C2->TXDR = reg_addr;

    // Wait for transfer complete of address
    while ((I2C2->ISR & I2C_ISR_TC) == 0);

    // Re-start condition for reading
    // Set slave address, number of bytes (len), read direction (1)
    I2C2->CR2 = (slave_addr << I2C_CR2_SADD_Pos) | (len << I2C_CR2_NBYTES_Pos) | I2C_CR2_START | I2C_CR2_RD_WRN;

    // Read data bytes
    for (i = 0; i < len; i++) {
        while ((I2C2->ISR & I2C_ISR_RXNE) == 0); // Wait for RXNE
        data[i] = I2C2->RXDR;
    }

    // Wait for transfer complete
    while ((I2C2->ISR & I2C_ISR_TC) == 0);

    // Generate STOP condition
    I2C2->CR2 |= I2C_CR2_STOP;

    while ((I2C2->ISR & I2C_ISR_STOPF) == 0);
    I2C2->ICR |= I2C_ICR_STOPCF;

    return 0; // Success
}

uint8_t I2C1_Master_Write(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    uint16_t i;
    slave_addr = slave_addr << 1;

    // Check for BUSY flag (optional, can be skipped for simplicity if you manage bus contention)
    while ((I2C2->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);

    // Set slave address (7-bit), number of bytes (1 for register address + len for data), write direction (0)
    I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    I2C2->CR2 = (slave_addr << I2C_CR2_SADD_Pos) | ( (1 + len) << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;

    // Send register address
    while ((I2C2->ISR & I2C_ISR_TXIS) == 0 && (I2C2->ISR & I2C_ISR_NACKF) == 0); // Wait for TXIS or NACKF
    if (I2C2->ISR & I2C_ISR_NACKF) {
        I2C2->ICR |= I2C_ICR_NACKCF; // Clear NACK flag
        I2C2->CR2 |= I2C_CR2_STOP;   // Generate STOP condition
        return 1; // NACK received
    }
    I2C2->TXDR = reg_addr;

    // Send data bytes
    for (i = 0; i < len; i++) {
        while ((I2C2->ISR & I2C_ISR_TXIS) == 0 && (I2C2->ISR & I2C_ISR_NACKF) == 0); // Wait for TXIS or NACKF
        if (I2C2->ISR & I2C_ISR_NACKF) {
            I2C2->ICR |= I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 1; // NACK received
        }
        I2C2->TXDR = data[i];
    }

    // Wait for transfer complete
    while ((I2C2->ISR & I2C_ISR_TC) == 0);

    // Generate STOP condition
    I2C2->CR2 |= I2C_CR2_STOP;

    // Wait for STOPF to be set (optional, good for checking if stop happened)
    while ((I2C2->ISR & I2C_ISR_STOPF) == 0);
    I2C2->ICR |= I2C_ICR_STOPCF; // Clear STOPF

    return 0; // Success
}


int
main()
{
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
//
  NVIC_SetPriority(GPDMA1_Channel0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 4));
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

  RCC->APB1LENR |= RCC_APB1LENR_I2C2EN;
  I2C2->CR1 &= ~I2C_CR1_PE; // Disable I2C peripheral
  I2C2->CR1 = 0;
  uint16_t SCLL = 999;
  uint16_t SCLH = 999;
  uint16_t PRESC = 124;
  uint16_t SDADEL = 1000;
  uint16_t SCLDEL = 999;
  I2C2->TIMINGR = (PRESC << I2C_TIMINGR_PRESC_Pos) |(SCLDEL << I2C_TIMINGR_SCLDEL_Pos) |(SDADEL << I2C_TIMINGR_SDADEL_Pos) | (SCLH << I2C_TIMINGR_SCLH_Pos) | SCLL;
//  I2C2->CR1 |= I2C_CR1_DNF;
  I2C2->CR2 = 0;
  I2C2->CR1 |= I2C_CR1_PE;
  uint8_t bq_config[1] = {1<<7 | 3 <<4 | 1 << 2};
  I2C1_Master_Write(0x6B,0x26,bq_config, 1);
  uint8_t data[16] = {};
//  uint16_t raw_reading = 0;
  float vsys = 0;
  float vbat = 0;



  while (1)
  {
    uart4_receive_to_idle();
    sleep_ms(1000);
    dbgLed.toggle();

    I2C1_Master_Read(0x6B, 0x30, data, 4);
//    raw_reading = (data[1] << 8 | data[0]) >> 1;
    vbat = ((data[1] << 8 | data[0]) >> 1) * 1.99;
    vsys = ((data[3] << 8 | data[2]) >> 1) * 1.99;
    static char message[128];//= "Hello world from CanSat!!!\r\n";
    snprintf(message, 128,"System: %f mV\r\nBattery: %f mV\r\n", vsys, vbat); // @suppress("Float formatting support")
    print((char*)message);
    if (gotMessage)
    {
      print((const char*) uart4_receive_buffer);
      uart4_receive_to_idle();
      gotMessage = 0;
    }
  }
}
