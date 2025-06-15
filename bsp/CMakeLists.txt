add_library(bsplib
  STATIC
	drivers/clock/clock.cpp
	drivers/gpio/gpio.cpp
	drivers/i2c/i2c.cpp
	drivers/sd/sd.cpp
)

target_include_directories(
  bsplib
  PUBLIC
	${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/CMSIS/Core/Include
	${CMAKE_SOURCE_DIR}/thirdparty/cmsis-device-h5/Include
	${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/STM32H5xx_HAL_Driver/Inc
)
