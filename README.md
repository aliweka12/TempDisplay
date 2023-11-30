# LCD Display and Temperature Reading Example

This example program demonstrates the use of an LCD display to show a simulated room temperature reading. It is designed for a specific hardware setup and utilizes peripherals including LTDC, DMA2D, and DSI.

## Features

- Initializes and configures the LCD display.
- Simulates temperature reading.
- Uses DMA2D for buffer management.
- Implements DSI callbacks for refreshing the display.

## Hardware Requirements

- STM32 microcontroller (or compatible hardware).
- LCD display compatible with the LTDC interface.
- LED for error indication.

## Software Requirements

- STM32 HAL library.
- BSP (Board Support Package) for the specific hardware used.

## Setup and Configuration

1. **Hardware Setup:** Ensure that the LCD display and LED are properly connected to the STM32 microcontroller.

2. **Project Configuration:** 
   - Include the HAL and BSP libraries specific to your hardware.
   - Ensure that the `main.h` file and the `image_320x240_argb8888.h` file are included in the project directory.

3. **Code Deployment:**
   - Compile and upload the code to your STM32 microcontroller using your preferred IDE and toolchain.

## Usage

After deploying the program to the microcontroller, the LCD display should initialize and show a default screen with a simulated temperature reading. The program will continuously refresh the temperature display.

## Code Structure

- `main.c`: Contains the main program logic including initialization and an infinite loop for continuous operation.
- `main.h`: Header file for the main source file.
- `image_320x240_argb8888.h`: Header file containing image data for the display.

## Functions

- `main()`: Initializes the hardware and enters the main program loop.
- `ReadTemperature()`: Simulates reading a temperature value.
- `HAL_DSI_EndOfRefreshCallback()`: Callback function for the DSI end of refresh.
- `SystemClock_Config()`: Configures the system clock.
- `LCD_Init()`: Initializes the LCD.
- `LTDC_Init()`: Initializes the LTDC peripheral.
- `LCD_BriefDisplay()`: Displays brief information on the LCD.
- `CopyBuffer()`: Copies data to the display buffer.
- `OnError_Handler()`: Error handler function.


