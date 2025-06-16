#include "diskio.h"
#include "stm32h5xx_hal.h"

extern SD_HandleTypeDef hsd1;

DSTATUS disk_initialize(BYTE pdrv) {
  if (pdrv != 0)
    return STA_NOINIT;
  // The disk initializatino is currently handled manually
  return RES_OK;
}

DSTATUS disk_status(BYTE pdrv) { return (pdrv == 0) ? 0 : STA_NOINIT; }

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv != 0)
    return RES_PARERR;
  if (HAL_SD_ReadBlocks(&hsd1, buff, sector, count, HAL_MAX_DELAY) != HAL_OK)
    return RES_ERROR;
  while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
    ;
  return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv != 0)
    return RES_PARERR;
  if (HAL_SD_WriteBlocks(&hsd1, (uint8_t *)buff, sector, count,
                         HAL_MAX_DELAY) != HAL_OK)
    return RES_ERROR;
  while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
    ;
  return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  if (pdrv != 0)
    return RES_PARERR;
  switch (cmd) {
  case CTRL_SYNC:
    return RES_OK;
  case GET_SECTOR_SIZE:
    *(WORD *)buff = 512;
    return RES_OK;
  case GET_BLOCK_SIZE:
    *(DWORD *)buff = 1;
    return RES_OK;
  case GET_SECTOR_COUNT: {
    HAL_SD_CardInfoTypeDef info;
    HAL_SD_GetCardInfo(&hsd1, &info);
    *(DWORD *)buff = info.LogBlockNbr;
    return RES_OK;
  }
  default:
    return RES_PARERR;
  }
}

DWORD get_fattime(void) {
  return ((DWORD)(2022 - 1980) << 25) // Year 2022
         | ((DWORD)1 << 21)           // Month January
         | ((DWORD)1 << 16)           // Day 1
         | ((DWORD)0 << 11)           // Hour 0
         | ((DWORD)0 << 5)            // Minute 0
         | ((DWORD)0 >> 1);           // Second / 2
}
