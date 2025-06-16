#pragma once

#include <stdint.h>
#include "etl/string.h"

#ifdef __cplusplus
extern "C" {
#endif

constexpr uint32_t UART4_RX_BUFFER_SIZE = 1023;

void print(const char* str);
void uart4_receive_to_idle();
bool uart4_data_received(etl::string<UART4_RX_BUFFER_SIZE+1>& str);
void uart4_setup();
void uart6_setup();


void __uart4_received_message(uint8_t*);

#ifdef __cplusplus
}
#endif
