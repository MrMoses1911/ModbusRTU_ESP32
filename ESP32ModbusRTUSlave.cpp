#include "ESP32ModbusRTUSlave.h"

// Modbus pinout configuration for MAX485 module
ModbusRTUSlave::ModbusRTUSlave(uint8_t dePin, uint8_t rePin, uint8_t rxPin, uint8_t txPin)  :
    _dePin(dePin), 
    _rePin(rePin), 
    _serial(&Serial2), 
    _rxPin(rxPin), 
    _txPin(txPin) {}

// Modbus initialzation 
void ModbusRTUSlave::begin(uint32_t baudRate, SerialConfig config, uint8_t slaveID, uint16_t numRegisters) {
    _config = config;
    _slaveID = slaveID;
    _numRegisters = numRegisters;
    _registers = new uint16_t[numRegisters]();

    uint8_t bitsPerCharacter = calculateBitsPerCharacter(_config);
    _charTimeout = (bitsPerCharacter * 1000000) / baudRate;

    pinMode(_dePin, OUTPUT);
    pinMode(_rePin, OUTPUT);

    _serial->begin(baudRate, config, _rxPin, _txPin);
    enableReceive();
}

// Calculation necessary to ensure the correct character delay during modbus packets
uint8_t ModbusRTUSlave::calculateBitsPerCharacter(SerialConfig config) {
    uint8_t bits = 1;

    switch (config) {
        case SERIAL_5N1: bits += 5; break; 
        case SERIAL_5N2: bits += 5; break; 
        case SERIAL_5E1: bits += 5; break; 
        case SERIAL_5E2: bits += 5; break; 
        case SERIAL_5O1: bits += 5; break; 
        case SERIAL_5O2: bits += 5; break;

        case SERIAL_6N1: bits += 6; break;
        case SERIAL_6N2: bits += 6; break; 
        case SERIAL_6E1: bits += 6; break; 
        case SERIAL_6E2: bits += 6; break; 
        case SERIAL_6O1: bits += 6; break; 
        case SERIAL_6O2: bits += 6; break;

        case SERIAL_7N1: bits += 7; break;
        case SERIAL_7N2: bits += 7; break;
        case SERIAL_7E1: bits += 7; break;
        case SERIAL_7E2: bits += 7; break; 
        case SERIAL_7O1: bits += 7; break;
        case SERIAL_7O2: bits += 7; break;

        case SERIAL_8N1: bits += 8; break; 
        case SERIAL_8N2: bits += 8; break; 
        case SERIAL_8E1: bits += 8; break; 
        case SERIAL_8E2: bits += 8; break; 
        case SERIAL_8O1: bits += 8; break; 
        case SERIAL_8O2: bits += 8; break;
    }

    if (config == SERIAL_5E1 || config == SERIAL_5E2 || config == SERIAL_5O1 || config == SERIAL_5O2 ||
        config == SERIAL_6E1 || config == SERIAL_6E2 || config == SERIAL_6O1 || config == SERIAL_6O2 ||
        config == SERIAL_7E1 || config == SERIAL_7E2 || config == SERIAL_7O1 || config == SERIAL_7O2 ||
        config == SERIAL_8E1 || config == SERIAL_8E2 || config == SERIAL_8O1 || config == SERIAL_8O2) {
        bits += 1;
    }

    if (config == SERIAL_5N2 || config == SERIAL_6N2 || config == SERIAL_7N2 || config == SERIAL_8N2 ||
        config == SERIAL_5E2 || config == SERIAL_6E2 || config == SERIAL_7E2 || config == SERIAL_8E2 ||
        config == SERIAL_5O2 || config == SERIAL_6O2 || config == SERIAL_7O2 || config == SERIAL_8O2) {
        bits += 2;
    } else {
        bits += 1;
    }

    return bits;
}

// function for sending data on the protocol
void ModbusRTUSlave::setRegister(uint16_t reg, uint16_t value) {
    if (reg < _numRegisters) {
        _registers[reg] = value;
    }
}

// function for receiving data on the protocol
uint16_t ModbusRTUSlave::getRegister(uint16_t reg) {
    return (reg < _numRegisters) ? _registers[reg] : 0;
}

// function that handles master's requests
void ModbusRTUSlave::handleRequest() {
    if (_serial->available()) {
        uint8_t frame[256];
        uint8_t length = 0;

        while (_serial->available()) {
            frame[length++] = _serial->read();
            delayMicroseconds(_charTimeout);
        }

        uint16_t receivedCRC = frame[length - 2] | (frame[length - 1] << 8);
        uint16_t calculatedCRC = calculateCRC(frame, length - 2);

        if (receivedCRC == calculatedCRC && frame[0] == _slaveID) {
            switch (frame[1]) {
                case 0x03:
                    processFunction03(frame, length);
                    break;
                case 0x06:
                    processFunction06(frame, length);
                    break;
                case 0x10:
                    processFunction16(frame, length);
                    break;                   
                default:
                    sendException(frame[1], 0x01);
            }
        }
    }
}

// function that enables the TX mode of the MAX485 module
void ModbusRTUSlave::enableTransmit() {
    digitalWrite(_dePin, HIGH);
    digitalWrite(_rePin, HIGH);
}

// function that enables the RX mode of the MAX485 module
void ModbusRTUSlave::enableReceive() {
    digitalWrite(_dePin, LOW);
    digitalWrite(_rePin, LOW);
}

// function that calculates the CRC validation
uint16_t ModbusRTUSlave::calculateCRC(uint8_t *buffer, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}   

// function that handles an exception request coming from the master
void ModbusRTUSlave::sendException(uint8_t functionCode, uint8_t exceptionCode) {
    uint8_t response[5];
    response[0] = _slaveID;
    response[1] = functionCode | 0x80;
    response[2] = exceptionCode;

    uint16_t crc = calculateCRC(response, 3);
    response[3] = crc & 0xFF;
    response[4] = crc >> 8;

    enableTransmit();
    _serial->write(response, 5);
    _serial->flush();
    enableReceive();
}

// Modbus 03 Function handler
void ModbusRTUSlave::processFunction03(uint8_t *frame, uint8_t length) {
    uint16_t startAddress = (frame[2] << 8) | frame[3];
    uint16_t quantity = (frame[4] << 8) | frame[5];

    if (startAddress + quantity <= _numRegisters) {
        uint8_t response[5 + quantity * 2];
        response[0] = _slaveID;
        response[1] = 0x03;
        response[2] = quantity * 2;

        for (uint16_t i = 0; i < quantity; i++) {
            response[3 + i * 2] = _registers[startAddress + i] >> 8;
            response[4 + i * 2] = _registers[startAddress + i] & 0xFF;
        }

        uint16_t crc = calculateCRC(response, 3 + quantity * 2);
        response[3 + quantity * 2] = crc & 0xFF;
        response[4 + quantity * 2] = crc >> 8;

        enableTransmit();
        _serial->write(response, 5 + quantity * 2);
        _serial->flush();
        enableReceive();
    } else {
        sendException(0x03, 0x02);
    }
}

// Modbus 06 Function handler
void ModbusRTUSlave::processFunction06(uint8_t *frame, uint8_t length) {
    uint16_t address = (frame[2] << 8) | frame[3];
    uint16_t value = (frame[4] << 8) | frame[5];

    if (address < _numRegisters) {
        _registers[address] = value;

        enableTransmit();
        _serial->write(frame, length);
        _serial->flush();
        enableReceive();
    } else {
        sendException(0x06, 0x02);
    }
}

// Modbus 16 Function handler
void ModbusRTUSlave::processFunction16(uint8_t *frame, uint8_t length) {
    uint16_t startAddress = (frame[2] << 8) | frame[3];
    uint16_t quantity = (frame[4] << 8) | frame[5];
    uint8_t byteCount = frame[6];

    if (startAddress + quantity > _numRegisters || byteCount != quantity * 2) {
        sendException(0x10, 0x02);
        return;
    }

    for (uint16_t i = 0; i < quantity; i++) {
        _registers[startAddress + i] = (frame[7 + i * 2] << 8) | frame[8 + i * 2];
    }

    uint8_t response[8];
    response[0] = _slaveID; 
    response[1] = 0x10;    
    response[2] = frame[2];
    response[3] = frame[3];
    response[4] = frame[4];
    response[5] = frame[5];

    uint16_t crc = calculateCRC(response, 6);
    response[6] = crc & 0xFF;
    response[7] = (crc >> 8) & 0xFF;

    enableTransmit();
    _serial->write(response, 8);
    _serial->flush();
    enableReceive();
}