set(GIT_DIR_LOOKUP_POLICY ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
add_subdirectory(etl)

set(STM32_HAL_SRC_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/STM32CubeH5/Drivers/STM32H5xx_HAL_Driver/Src/)
set(STM32_HAL_INC        ${CMAKE_CURRENT_SOURCE_DIR}/STM32CubeH5/Drivers/STM32H5xx_HAL_Driver/Inc)

set(STM32_HAL_SD_SRC
  hal_mock.c
  ${STM32_HAL_SRC_PREFIX}stm32h5xx_hal_sd.c
  ${STM32_HAL_SRC_PREFIX}stm32h5xx_hal_sd_ex.c
  ${STM32_HAL_SRC_PREFIX}stm32h5xx_hal_sdio.c
  ${STM32_HAL_SRC_PREFIX}stm32h5xx_hal_mmc.c
  ${STM32_HAL_SRC_PREFIX}stm32h5xx_hal_mmc_ex.c
  ${STM32_HAL_SRC_PREFIX}stm32h5xx_ll_sdmmc.c
)


add_library(
  stm32hal
  STATIC
  ${STM32_HAL_SD_SRC}
)

target_include_directories(
  stm32hal
  PUBLIC
  ${STM32_HAL_INC}
  ${CMAKE_SOURCE_DIR}/thirdparty
  ${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/CMSIS/Core/Include
  ${CMAKE_SOURCE_DIR}/thirdparty/cmsis-device-h5/Include
)

