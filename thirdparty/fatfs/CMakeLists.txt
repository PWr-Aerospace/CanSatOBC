set(FATFS_SRC_PREFIX "source/")

set(FATFS_SRC
  ${FATFS_SRC_PREFIX}diskio.c
  ${FATFS_SRC_PREFIX}diskio.h
  ${FATFS_SRC_PREFIX}ff.c
  ${FATFS_SRC_PREFIX}ff.h
  ${FATFS_SRC_PREFIX}ffconf.h
  ${FATFS_SRC_PREFIX}ffsystem.c
  ${FATFS_SRC_PREFIX}ffunicode.c
)

add_library(
  fatfs
  STATIC
  ${FATFS_SRC}
)
target_include_directories(fatfs PRIVATE
	${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/STM32H5xx_HAL_Driver/Inc
  ${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/CMSIS/Core/Include
  ${CMAKE_SOURCE_DIR}/thirdparty/cmsis-device-h5/Include
  ${CMAKE_SOURCE_DIR}/thirdparty
)

target_compile_options(fatfs PRIVATE
    -Wno-error=implicit-function-declaration
    -Wno-error=unused-but-set-variable
    -Wno-error=maybe-uninitialized
    -Wno-error=unused-variable
)
