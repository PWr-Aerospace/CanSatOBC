#include "config.h"

#include "drivers/clock/clock.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/i2c/i2c.hpp"
#include "drivers/sd/sd.hpp"
#include "drivers/uart/uart.hpp"
#include "proxy/logger/logger.hpp"

#include "etl/string.h"
#include "stm32h533xx.h"
#include "system/system.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int
main()
{
  clock_setup_HSE(16, desiredMhz, 1, 1);
  SystemCoreClockUpdate();

  Gpio dbgLed('C', 13, Mode::Out);
  Gpio mux('A', 10, Mode::Out);
  Gpio xbeeRst('A', 4, Mode::Out);
  Gpio uart4_tx('D', 12, Mode::AF, Type::OD, 8);
  Gpio uart4_rx('D', 11, Mode::AF, Type::OD, 8);
  Gpio usart6_tx('C', 6, Mode::AF, Type::OD, 7);
  Gpio usart6_rx('C', 7, Mode::AF, Type::OD, 7);
  Gpio scl('B', 10, Mode::AF, Type::OD, 4);
  Gpio sda('B', 12, Mode::AF, Type::OD, 4);
  Gpio sd_d0('C', 8, Mode::AF, Type::PP, Pull::Up, Speed::VeryHigh, 12);
  Gpio sd_d1('C', 9, Mode::AF, Type::PP, Pull::Up, Speed::VeryHigh, 12);
  Gpio sd_d2('C', 10, Mode::AF, Type::PP, Pull::Up, Speed::VeryHigh, 12);
  Gpio sd_d3('C', 11, Mode::AF, Type::PP, Pull::Up, Speed::VeryHigh, 12);
  Gpio sd_clk('C', 12, Mode::AF, Type::PP, Pull::None, Speed::VeryHigh, 12);
  Gpio sd_cmd('D', 2, Mode::AF, Type::PP, Pull::None, Speed::VeryHigh, 12);

  xbeeRst.set();
  mux.set();

  uart4_setup();

  I2C2_setup();
  uint8_t bq_config[1] = {1 << 7 | 3 << 4 | 1 << 2};
  I2C2_Master_Write(0x6B, 0x26, bq_config, 1);
  uint8_t data[16] = {};
  float vsys = 0;
  float vbat = 0;

  sd_init();
  // Enable write access to BKPSRAM
  PWR->DBPCR |= PWR_DBPCR_DBP;
  // Enable voltage regulator for backup domain
  PWR->BDCR |= PWR_BDCR_BREN;
//  uint32_t myvar;
  struct backup{
	  uint32_t a;
	  uint32_t b;
  };
  auto mem = reinterpret_cast<backup*>(BKPSRAM_BASE);
//  mem->a = 2137;
//  mem->b = 0xDEAD;

  list_root_directory();

  etl::string<UART4_RX_BUFFER_SIZE+1> str;

  while (1)
  {
    dbgLed.toggle();

    list_root_directory();

    I2C2_Master_Read(0x6B, 0x30, data, 4);
    //    raw_reading = (data[1] << 8 | data[0]) >> 1;
    vbat = ((data[1] << 8 | data[0]) >> 1) * 1.99;
    vsys = ((data[3] << 8 | data[2]) >> 1) * 1.99;
    // static char message[128]; //= "Hello world from CanSat!!!\r\n";
    printf("System: %f mV\r\nBattery: %f mV\r\nmyvar: %ld\r\n",
           vsys,
           vbat,
		   mem->a);

    if(uart4_data_received(str)){
    	printf("Got message: '%s'\r\n", str.c_str());
    }
    sleep_ms(1000);
  }
}
