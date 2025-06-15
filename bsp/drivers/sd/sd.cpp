#include "sd.hpp"

#include "stm32h533xx.h"

#define SDIO_MAX_IO_NUMBER 7U

#include "stm32h5xx_hal_sd.h"
#include "stm32h5xx_hal_sdio.h"

SD_HandleTypeDef hsd1;

void
sd_init()
{
  RCC->AHB4ENR |= RCC_AHB4ENR_SDMMC1EN;

  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B; // Start with 1-bit
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 3;
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    while (1)
      ;
  }
}

// #include "ff.h"
// #include <stdio.h>
//
// void
// list_root_directory(void)
// {
// }
