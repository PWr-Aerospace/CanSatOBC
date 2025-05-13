# CanSat OBC
This repository holds code for the CanSatOBC based on STM32H533.

## Quickstart
To clone the repository, run following commands:
```bash
git clone 
cd CanSatOBC
git submodule update --init
```

## Building the project
This project is based on cmake toolchain. There is no CubeMX project and therefore no `.ioc` file.
### STM32CubeIDE
The CubeIDE should be able to pick up the project if you simply open it using `Open Projects from File System`. Make sure to first unselect all projects and then select only `CanSatOBC` project, which should be at the top of the list.
By default, the CubeIDE will pick up also projects located in `thirdparty` directory, which is unwanted in our case.
### CLI (Linux)
In order to build the project from terminal in linux, ensure you have `cmake` and proper compiler (`arm-gcc-none-eabi`) installed.
Then simply run following command in the root directory of the project:
```bash
mkdir build && cmake -B build && cmake --build build
```
### CLion
In order to set up CLion, follow [this guide](clion.md).

## Project structure
The structure contains board specific code in `bsp` directory and the entrypoint is located in `src/main.cpp` file.
The libraries such as CMSIS and HAL are managed using submodules and located in `thirdparty` folder. The toolchain used by the CubeIDE can be found in root directory, with an alternative located in `cmake` folder.
The structure looks as follows:
```bash
|-- bsp
|    |-- drivers
|    |    |-- clock
|    |    |-- gpio
|    |    |-- serial
|    |    |-- spi
|    |    +-- etc.
|    |-- proxy
|    |    |-- BMP280 
|    |    |-- MPU9250 
|    |    +-- BQ25622
|    +-- system
|         |-- startup_stm32h533vetx.s
|         |-- syscalls.c
|         +-- sysmem.c
|-- cmake
|-- src
|    +-- main.cpp
+-- thirdparty
     |-- cmsis-device-h5
     +-- STM32CubeH5
```
