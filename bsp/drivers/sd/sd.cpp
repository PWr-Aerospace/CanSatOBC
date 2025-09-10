#include "sd.hpp"
#include "ff.h"

#include "proxy/logger/logger.hpp"

#include "stm32h533xx.h"

#define SDIO_MAX_IO_NUMBER 7U
#include "stm32h5xx_hal_sd.h"
#include "stm32h5xx_hal_sdio.h"

SD_HandleTypeDef hsd1;

enum class SdState
{
  UNINITIALIZED,
  OK,
  ERROR
};

static SdState state;

void zero_fatfs_table(void);
void fatfs_setup();


void
sd_init()
{
	static bool initialized = false;
	if(initialized)
		return;
  RCC->AHB4ENR |= RCC_AHB4ENR_SDMMC1EN;

  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 3;
  if (HAL_SD_Init(&hsd1) == HAL_OK)
  {
    state = SdState::OK;
    zero_fatfs_table();
    fatfs_setup();
    initialized = true;
  }
}

bool
is_sd_ok()
{
  return state == SdState::OK;
}

extern FATFS* FatFs[]; // Or declare it yourself if needed

void
zero_fatfs_table(void)
{
  for (int i = 0; i < FF_VOLUMES; i++)
  {
    FatFs[i] = NULL;
  }
}
static FATFS fs;

void
fatfs_setup()
{
  FRESULT res;
  zero_fatfs_table();
  res = f_mount(&fs, "", 0); // "" = default drive, 1 = mount now
  if (res != FR_OK)
  {
    printf("f_mount failed: %d\r\n", res);
    return;
  }
}

void
list_root_directory(void)
{
  if (!is_sd_ok())
  {
    printf("Sd in bad state");
    return;
  }

  FRESULT res;
  DIR dir;
  FILINFO fno;

  res = f_opendir(&dir, "/"); // Open root directory
  if (res != FR_OK)
  {
    printf("Failed to open root directory: %d\r\n", res);
    return;
  }

  printf("Files in root directory:\r\n");

  while (1)
  {
    res = f_readdir(&dir, &fno); // Read next item
    if (res != FR_OK || fno.fname[0] == 0)
      break; // Break on error or end of dir

    if (fno.fattrib & AM_DIR)
    {
      printf("  [DIR]  '%s'\r\n", (char*) fno.fname);
    }
    else
    {
      printf("  [FILE] '%s' (%lu bytes)\r\n", (char*) fno.fname, (unsigned long) fno.fsize);
    }
  }

  res = f_closedir(&dir);
  if (res != FR_OK)
  {
    printf("Failed to close root directory: %d\r\n", res);
    return;
  }
}
