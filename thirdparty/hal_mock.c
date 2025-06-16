#include "hal_mock.h"

uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint64_t PeriphClk)
{
  switch (PeriphClk)
      {
  case RCC_PERIPHCLK_SDMMC1:
	  return 48000000;
    break;
  default:
	  while(1)
		  ;
	  break;
      }
  return 0;
}
