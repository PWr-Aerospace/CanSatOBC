#pragma once

#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>

void SPI4_setup();
void SPI4_Transmit_HalfDuplex(uint8_t *data, uint16_t size);
void SPI4_Receive_HalfDuplex(uint8_t *data, uint16_t size);

void SPI3_setup();
uint8_t* SPI3_xfer(uint8_t* data, uint8_t size);

#ifdef __cplusplus
}
#endif
