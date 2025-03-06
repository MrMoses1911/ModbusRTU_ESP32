# ModbusRTU_ESP32 Library

The **ModbusRTU_ESP32 Library** enables any ESP32 to function as a Modbus RTU slave device using the RS-485 protocol. This library is specifically designed for industrial applications where communication with PLCs, HMIs, or other Modbus master devices is required.

## Features
- Supports RS-485 communication via the MAX 485 or MAX 1487 transceivers.
- Configurable UART pins for flexibility in hardware design.
- Implements the following Modbus RTU functions:
	- **03 (Read Holding Registers):** Read multiple registers.
	- **06 (Write Single Register):** Write to a single register.
	- **16 (Write Multiple Registers):** Write to multiple registers.
- Dynamic configuration of serial settings, including:
	- Baud rates.
	- Parity (None, Even, Odd).
	- Data bits (5, 6, 7, or 8).
	- Stop bits (1 or 2).
- Easy-to-use API for reading and writing registers.

## Hardware Requirements
- **ESP32 or Arduino Based Development Board** for running the library.
- **MAX 485/1487 or Equivalent RS-485 Transceiver** for RS-485 communication.
- Pull-up and pull-down resistors, and a 120-ohm termination resistor, if needed.

## Installation
Download or clone this repository.
Copy the folder ModbusRTU_ESP32 to your Arduino libraries directory:
```
<Arduino Installation Directory>/libraries/
```
Restart the Arduino IDE to apply changes.

## Basic Usage
*The Modbus communication was tested using a **WAGO Compact Controller 100** plc as a master device.*

```cpp
#include <ModbusRTU_ESP32.h>

ModbusRTU_ESP32 modbus(GPIO_NUM_47, GPIO_NUM_48, GPIO_NUM_33, GPIO_NUM_34);

uint16_t readData, writeData;

void setup() {
	//baudrate, bit configuration, slave id, number of registers
    modbus.beginRTU(9600, SERIAL_8E1, 1, 2);
}

void loop() {
	//Updates de Modbus communication
    modbus.handleRequest();

	//Set the register 0 with the "writeData" value
	writeData = 155;
    modbus.setRegister(0, writeData);

	//Read the register 1 value and stores it in the readData variable
    readData = modbus.getRegister(1);
}
```

## Examples
Check the **examples/** folder for ready-to-use sketches, including:
Basic usage for reading and writing registers.
Advanced configuration with different UART settings.

## Compatibility
This library is compatible with:
Arduino IDE 1.8.x and newer.

## License
This project is licensed under the MIT License. Feel free to use, modify, and distribute it.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request for improvements or bug fixes.