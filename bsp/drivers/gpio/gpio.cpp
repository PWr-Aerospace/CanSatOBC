#include "gpio.hpp"

#include "stm32h533xx.h"

constexpr GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};

void
assert(bool result)
{
  if (!result)
  {
    while (1)
      ;
  }
}

Gpio::Gpio(char port, uint8_t pin, Mode m, uint8_t AF)
{
  assert(port >= 'A');
  assert(port <= 'F');
  assert(pin <= 15);

  _port = port - 'A';
  _pin = pin;
  _mode = m;

  RCC->AHB2ENR |= (1 << _port);

  // Clear mode register and then write wanted value
  reinterpret_cast<GPIO_TypeDef*>(ports[_port])->MODER &= ~(3 << (_pin * 2));
  reinterpret_cast<GPIO_TypeDef*>(ports[_port])->MODER |=
      (static_cast<uint8_t>(_mode) << (_pin * 2));

  if (_mode == Mode::AF)
  {
    if (_pin > 7)
      ports[_port]->AFR[1] |= (AF << 4 * (_pin - 8));
    else
      ports[_port]->AFR[0] |= (AF << (4 * _pin));
  }
}

void
Gpio::set()
{
  reinterpret_cast<GPIO_TypeDef*>(ports[_port])->BSRR |= 1 << _pin;
}

void
Gpio::reset()
{
  reinterpret_cast<GPIO_TypeDef*>(ports[_port])->BSRR |= 1 << (_pin + 16);
}

bool
Gpio::get()
{
  return reinterpret_cast<GPIO_TypeDef*>(ports[_port])->IDR & (1 << _pin);
}

void
Gpio::toggle()
{
  if (get())
    reset();
  else
    set();
}
