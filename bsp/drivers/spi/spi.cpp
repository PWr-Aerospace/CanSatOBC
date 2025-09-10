#include "spi.hpp"

#include "stm32h533xx.h"

uint8_t*
SPI3_xfer(uint8_t* data, uint8_t size)
{
  // Set SPI to transmit mode
  SPI3->CR1 &= ~SPI_CR1_SPE;
  SPI3->CR2 = size;
  SPI3->CR1 |= SPI_CR1_SPE;
  static uint8_t recv[128];

  for (uint16_t i = 0; i < size; i++)
  {
    SPI3->TXDR = data[i];
    recv[i] = SPI3->RXDR;
    //	        while (!(SPI4->SR & SPI_SR_TXC)); // Wait for transfer completion
  }
  return recv;
}

void
SPI4_Receive_HalfDuplex(uint8_t* data, uint16_t size)
{
  SPI4->CR1 &= ~SPI_CR1_SPE;
  SPI4->CR1 &= ~SPI_CR1_HDDIR; // BIDIOE = 0 for receive
  SPI4->CR2 = size;
  SPI4->CR1 |= SPI_CR1_SPE;

  for (uint16_t i = 0; i < size; i++)
  {
    SPI4->CR1 |= SPI_CR1_HDDIR;
    SPI4->TXDR = 0xFF;
    SPI4->CR1 &= ~SPI_CR1_HDDIR;
    while (!(SPI4->SR & SPI_SR_RXP))
      ;
    data[i] = SPI4->RXDR;
  }
}

void
SPI4_Transmit_HalfDuplex(uint8_t* data, uint16_t size)
{
  SPI4->CR1 &= ~SPI_CR1_SPE;
  SPI4->CR1 |= SPI_CR1_HDDIR; // Set transmitte
  SPI4->CR2 = size;
  SPI4->CR1 |= SPI_CR1_SPE;

  for (uint16_t i = 0; i < size; i++)
  {
    SPI4->TXDR = data[i];
  }
}

void
SPI4_setup()
{
  RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;

  RCC->CCIPR3 &= ~RCC_CCIPR3_SPI4SEL;
  SPI4->CR1 &= ~SPI_CR1_SPE; // Clear SPE bit using the defined bit mask
  SPI4->CR1 = 0;             // Reset CR1 for clean configuration

  SPI4->CFG1 = 0;
  SPI4->CFG2 = 0;
  SPI4->CFG1 &= ~(SPI_CFG1_MBR | SPI_CFG1_DSIZE);
  SPI4->CFG1 |= (1 << SPI_CFG1_MBR_Pos) | (7 << SPI_CFG1_DSIZE_Pos);

  SPI4->CFG2 |= (1 << SPI_CFG2_AFCNTR_Pos) | SPI_CFG2_MASTER | (0b11 << SPI_CFG2_COMM_Pos);

  // 4. Enable SPI4
  SPI4->IFCR |= SPI_IFCR_MODFC;
  SPI4->CR1 |= SPI_CR1_SPE;
}

void
SPI3_setup()
{
  RCC->APB1LENR |= RCC_APB1LENR_SPI3EN;
  SPI3->CR1 &= ~SPI_CR1_SPE; // Clear SPE bit using the defined bit mask
  SPI3->CR1 = 0;             // Reset CR1 for clean configuration
  SPI3->CFG1 &= ~(SPI_CFG1_MBR | SPI_CFG1_DSIZE);
  SPI3->CFG1 |= (0b100 << SPI_CFG1_MBR_Pos) | (7 << SPI_CFG1_DSIZE_Pos);
  SPI3->CFG2 |= (1 << SPI_CFG2_MASTER_Pos);
  SPI3->CR1 |= SPI_CR1_SPE;
}
