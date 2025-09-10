cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cubeide-gcc.cmake)

###################### CONSTANTS ######################################
set (MCPU_CORTEX_M33				  "-mcpu=cortex-m33")
set (MFPU_FPV4_SP_D16                 "-mfpu=fpv4-sp-d16")
set (MFPU_FPV5_D16                    "-mfpu=fpv5-d16")
set (RUNTIME_LIBRARY_REDUCED_C        "--specs=nano.specs")
set (RUNTIME_LIBRARY_STD_C            "")
set (RUNTIME_LIBRARY_SYSCALLS_MINIMAL "--specs=nosys.specs")
set (RUNTIME_LIBRARY_SYSCALLS_NONE    "")
set (MFLOAT_ABI_SOFTWARE              "-mfloat-abi=soft")
set (MFLOAT_ABI_HARDWARE              "-mfloat-abi=hard")
set (MFLOAT_ABI_MIX                   "-mfloat-abi=softfp")
#######################################################################

###################### VARIABLES ######################################
set (PROJECT_NAME             "CanSatOBC")
set (LINKER_SCRIPT            "../STM32H533VETX_FLASH.ld")
set (MCPU                     "-mcpu=Cortex-M33")
set (MFPU                     "-mfpu=fpv5-sp-d16")
set (MFLOAT_ABI               "")
set (RUNTIME_LIBRARY          "--specs=nano.specs")
set (RUNTIME_LIBRARY_SYSCALLS "--specs=nosys.specs")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (PROJECT_DEFINES STM32H533xx)

################## PROJECT SETUP ######################################
project(${PROJECT_NAME})
enable_language(ASM)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_C_FLAGS "${MCPU} -std=gnu11 ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Werror")
set(CMAKE_CXX_FLAGS "${MCPU} -std=c++20 ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Werror")
set(CMAKE_EXE_LINKER_FLAGS "-T${LINKER_SCRIPT} ${RUNTIME_LIBRARY_SYSCALLS} -Wl,-Map=test.map -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group -u _printf_float")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")

add_compile_definitions (${PROJECT_DEFINES})

add_subdirectory(bsp)
add_subdirectory(thirdparty)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/interrupts.cpp
  bsp/system/syscalls.c
  bsp/system/sysmem.c
  bsp/system/system_stm32h5xx.c
  bsp/system/startup_stm32h533vetx.s
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
	${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/CMSIS/Core/Include
	${CMAKE_SOURCE_DIR}/thirdparty/cmsis-device-h5/Include
	${CMAKE_SOURCE_DIR}/thirdparty/etl/include
	${CMAKE_SOURCE_DIR}/thirdparty/STM32CubeH5/Drivers/STM32H5xx_HAL_Driver/Inc
	${CMAKE_SOURCE_DIR}/thirdparty
  bsp
)

target_link_libraries(
  ${PROJECT_NAME}
  bsplib
  stm32hal
  etl::etl
  fatfs
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  bsp
  bsp/thirdparty/etl/include
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)

