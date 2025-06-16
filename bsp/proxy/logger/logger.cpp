#include "logger.hpp"

#include "drivers/uart/uart.hpp"

#include <stdint.h>
#include <stdio.h>

void print(const char* str);

int
printf(const char* format, ...)
{
  constexpr uint32_t PRINTF_BUFFER_SIZE = 8192;
  char buffer[PRINTF_BUFFER_SIZE];
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, args);
  va_end(args);

  // Truncate if buffer is full
  buffer[PRINTF_BUFFER_SIZE - 1] = '\0';

  print(buffer);
  return len;
}
