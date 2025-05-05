#pragma once

#include <stdint.h>

enum class Mode : uint8_t
{
  In = 0,
  Out = 1,
  AF = 2,
  Analog = 3
};

enum class Type : uint8_t
{
  PP = 0,
  OD = 1
};

class Gpio
{
public:
  Gpio(char port, uint8_t pin, Mode m, uint8_t AF = 0);
  bool get();
  void set();
  void reset();
  void toggle();

private:
  uint8_t _port{};
  uint8_t _pin{};
  Mode _mode;
};
