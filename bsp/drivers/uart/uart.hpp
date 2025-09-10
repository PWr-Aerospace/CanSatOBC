#pragma once

#include "etl/queue.h"
#include "etl/string.h"
#include "interrupts.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

class Uart
{
  constexpr static uint32_t max_messages_no = 16;

public:
  constexpr static uint32_t RX_BUFFER_SIZE = 1024;
  Uart(uint8_t port_id, int interruptId);

  void InterruptHandler(const size_t id);

  bool data_received(etl::string<RX_BUFFER_SIZE>& str);
  void receive_to_idle();

  // Callback for the interrupt handler.
  etl::function_mp<Uart, size_t, &Uart::InterruptHandler> callback;

  //constexpr uint32_t UART4_RX_BUFFER_SIZE = 1023;
  uint8_t uart4_receive_buffer[RX_BUFFER_SIZE];

private:
  etl::queue<etl::string<RX_BUFFER_SIZE>, max_messages_no> _queue;
  uint8_t port_id;
  uint8_t _raw_rx[RX_BUFFER_SIZE];
};

void print(const char* str);
void uart4_receive_to_idle();
// bool uart4_data_received(etl::string<UART4_RX_BUFFER_SIZE + 1>& str);
void uart4_setup();
void uart6_setup();

void __uart4_received_message(uint8_t*);

#ifdef __cplusplus
}
#endif
