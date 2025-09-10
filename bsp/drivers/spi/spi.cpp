#include "spi.hpp"

#include "stm32h533xx.h"


uint8_t* SPI3_xfer(uint8_t* data, uint8_t size) {
	// Set SPI to transmit mode
		SPI3->CR1 &= ~SPI_CR1_SPE;
	    SPI3->CR2 = size;
	    SPI3->CR1 |= SPI_CR1_SPE;
	    static uint8_t recv[128];

	    for (uint16_t i = 0; i < size; i++) {
	        SPI3->TXDR = data[i];
	        recv[i] = SPI3->RXDR;
//	        while (!(SPI4->SR & SPI_SR_TXC)); // Wait for transfer completion
	    }
	    return recv;
}

void SPI4_Receive_HalfDuplex(uint8_t *data, uint16_t size) {

	// Set SPI to receive mode
	SPI4->CR1 &= ~SPI_CR1_SPE;
	SPI4->CR1 &= ~SPI_CR1_HDDIR; // BIDIOE = 0 for receive
	SPI4->CR2 = size;
	SPI4->CR1 |= SPI_CR1_SPE;

	    // Initiate reception (e.g., by writing dummy data for master to generate clocks)
	    // As mentioned, for half-duplex receive, if the slave needs clocks from the master,
	    // the master will perform dummy writes to SPI4->DR to generate those clocks.
	    // However, the data read will come from the slave.

	    for (uint16_t i = 0; i < size; i++) {
	        // Dummy write to generate clock pulses (if your slave needs them to send data)
	        // Ensure you toggle BIDIOE back to transmit for *this specific dummy write* if needed
	        // but typically, once in RX mode, you just wait for RXNE after the initial setup.
	        // If your slave simply starts sending data when NSS is asserted, no dummy write is needed.
	        // If your slave waits for a master clock, a dummy write would be done here:
	    	SPI4->CR1 |= SPI_CR1_HDDIR;
	    	SPI4->TXDR = 0xFF;
	        SPI4->CR1 &= ~SPI_CR1_HDDIR;
	        // while (!(SPI4->SR & SPI_SR_TXE)); // Wait for dummy transmit to start clock generation

	        // Wait until receive buffer is not empty
//	        while (!(SPI4->SR & SPI_SR_EOT)); // Wait for transfer completion
	    	while(!(SPI4->SR & SPI_SR_RXP))
	    		;
	        // Read data from the data register
	        data[i] = SPI4->RXDR;
	    }

	    // Wait until not busy (if applicable after reading all data)
//	    while (!(SPI4->SR & SPI_SR_EOT)); // Wait for transfer completion

}


void SPI4_Transmit_HalfDuplex(uint8_t *data, uint16_t size) {
	// Set SPI to transmit mode
	SPI4->CR1 &= ~SPI_CR1_SPE;
	    SPI4->CR1 |= SPI_CR1_HDDIR; // Set transmitte
	    SPI4->CR2 = size;
	    SPI4->CR1 |= SPI_CR1_SPE;

	    for (uint16_t i = 0; i < size; i++) {
	        // Wait until transmit buffer is empty


	        // Write data to the data register
	        SPI4->TXDR = data[i];
//	        while (!(SPI4->SR & SPI_SR_TXC)); // Wait for transfer completion
	    }

	    // Wait until transmission is complete (BSY flag clear)
//	    while (!(SPI4->SR & SPI_SR_TXC)); // Wait for transfer completion
}


void SPI4_setup(){
	RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;

	RCC->CCIPR3 &= ~RCC_CCIPR3_SPI4SEL;
	// Just use directly HSE as clk source
//	RCC->CCIPR3 |= (0b101 << RCC_CCIPR3_SPI4SEL_Pos);
	// Half duplex mode
//	SPI4->CFG2 |= (0b11 << SPI_CFG2_COMM_Pos);
	// 1. Disable SPI4 to configure it
	    SPI4->CR1 &= ~SPI_CR1_SPE; // Clear SPE bit using the defined bit mask

	    // 2. Configure CR1
	    SPI4->CR1 = 0; // Reset CR1 for clean configuration

	    // Master mode (MSTR)
	    SPI4->CFG1 = 0;
	    SPI4->CFG2 = 0;
	    SPI4->CFG1 &= ~(SPI_CFG1_MBR |  SPI_CFG1_DSIZE);
	    SPI4->CFG1 |= (1 << SPI_CFG1_MBR_Pos) | (7 << SPI_CFG1_DSIZE_Pos);

	    // Half-duplex mode (BIDIMODE)
	    SPI4->CFG2 |= (1 << SPI_CFG2_AFCNTR_Pos) | SPI_CFG2_MASTER | (0b11 << SPI_CFG2_COMM_Pos);


	    // 4. Enable SPI4
	    SPI4->IFCR |= SPI_IFCR_MODFC;
	    SPI4->CR1 |= SPI_CR1_SPE;

}


void SPI3_setup(){
	RCC->APB1LENR |= RCC_APB1LENR_SPI3EN;

//	RCC->CCIPR3 &= ~RCC_CCIPR3_SPI4SEL;
	// Just use directly HSE as clk source
//	RCC->CCIPR3 |= (0b101 << RCC_CCIPR3_SPI4SEL_Pos);
	// Half duplex mode
//	SPI4->CFG2 |= (0b11 << SPI_CFG2_COMM_Pos);
	// 1. Disable SPI4 to configure it
	    SPI3->CR1 &= ~SPI_CR1_SPE; // Clear SPE bit using the defined bit mask

	    // 2. Configure CR1
	    SPI3->CR1 = 0; // Reset CR1 for clean configuration

	    // Master mode (MSTR)
//	    SPI3->CFG1 = 0;
//	    SPI3->CFG2 = 0;
	    SPI3->CFG1 &= ~(SPI_CFG1_MBR |  SPI_CFG1_DSIZE);
	    SPI3->CFG1 |= (0b100 << SPI_CFG1_MBR_Pos) | (7 << SPI_CFG1_DSIZE_Pos);

	    // Half-duplex mode (BIDIMODE)
	    SPI3->CFG2 |= (1 << SPI_CFG2_MASTER_Pos);


	    // 4. Enable SPI4
//	    SPI3->IFCR |= SPI_IFCR_MODFC;
	    SPI3->CR1 |= SPI_CR1_SPE;

}

