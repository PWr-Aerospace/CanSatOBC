#include "i2c.hpp"

#include "stm32h533xx.h"

void I2C2_setup(){
	  RCC->APB1LENR |= RCC_APB1LENR_I2C2EN;
	  I2C2->CR1 &= ~I2C_CR1_PE;
	  I2C2->CR1 = 0;
	  // TODO: Correct these values but for know it works
	  uint16_t SCLL = 999;
	  uint16_t SCLH = 999;
	  uint16_t PRESC = 124;
	  uint16_t SDADEL = 1000;
	  uint16_t SCLDEL = 999;
	  I2C2->TIMINGR = (PRESC << I2C_TIMINGR_PRESC_Pos) |(SCLDEL << I2C_TIMINGR_SCLDEL_Pos) |(SDADEL << I2C_TIMINGR_SDADEL_Pos) | (SCLH << I2C_TIMINGR_SCLH_Pos) | SCLL;
//	  I2C2->CR1 |= I2C_CR1_DNF;
	  I2C2->CR2 = 0;
	  I2C2->CR1 |= I2C_CR1_PE;
}

uint8_t I2C2_Master_Read(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
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

uint8_t I2C2_Master_Write(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
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
