# ModbusRTU_MAX485 Library

The **ModbusRTU_MAX485 Library** enables any ESP32 or Arduino based development board to function as a Modbus RTU slave device using the RS-485 protocol. This library is specifically designed for industrial applications where communication with PLCs, HMIs, or other Modbus master devices is required.

## Features
- Supports RS-485 communication via the MAX485 transceiver.
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
- Fully compliant with the Modbus RTU protocol specifications.

## Hardware Requirements
- **ESP32 or Arduino Based Development Board** for running the library.
- **MAX485 or Equivalent RS-485 Transceiver** for RS-485 communication.
- Pull-up and pull-down resistors, and a 120-ohm termination resistor, if needed.

## Installation
Download or clone this repository.
Copy the folder ModbusRTU_MAX485 to your Arduino libraries directory:
```
<Arduino Installation Directory>/libraries/
```
Restart the Arduino IDE to apply changes.

## Basic Usage
```cpp
#include "ModbusRTU_MAX485.h"

// Configure the DE and RE pins (control MAX485), RX, and TX pins
ModbusRTU_MAX485 modbus(4, 5, 16, 17);

void setup() {
    modbus.beginRTU(9600, SERIAL_8N1, 1, 10); // 9600 baud, no parity, slave ID 1, 10 registers
    modbus.setRegister(0, 1234);          // Set initial value of register 0
}

void loop() {
    modbus.handleRequest(); // Process Modbus requests from the master
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