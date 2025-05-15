#pragma once
#include <stdint.h>

#define RCC_PERIPHCLK_SDMMC1           ((uint64_t)0x00400000U)

uint32_t          HAL_RCCEx_GetPeriphCLKFreq(uint64_t PeriphClk);
