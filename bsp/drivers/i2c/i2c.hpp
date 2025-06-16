#pragma once

#include <stdint.h>

void I2C2_setup();
uint8_t I2C2_Master_Read(uint8_t slave_addr, uint8_t reg_addr, uint8_t* data, uint16_t len);
uint8_t I2C2_Master_Write(uint8_t slave_addr, uint8_t reg_addr, uint8_t* data, uint16_t len);
