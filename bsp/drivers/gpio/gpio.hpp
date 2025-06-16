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

enum class Speed : uint8_t
{
  Low = 0,
  Medium = 1,
  High = 2,
  VeryHigh = 3
};

enum class Pull : uint8_t
{
  None = 0,
  Up = 1,
  Down = 2
};

// Gpio uart4_tx('D', 12, Mode::AF, Type::OD, 8);
class Gpio
{
public:
  Gpio(char port, uint8_t pin, Mode m, Type t = Type::PP, uint8_t AF = 0);
  Gpio(char port, uint8_t pin, Mode m, Type t, Pull p, Speed s = Speed::Low, uint8_t AF = 0);
  bool get();
  void set();
  void reset();
  void toggle();

private:
  void setup();
  uint8_t _port{};
  uint8_t _pin{};
  Mode _mode;
  Type _type;
  Pull _p{Pull::None};
  Speed _s{Speed::Low};
  uint8_t _af = 0;
};
