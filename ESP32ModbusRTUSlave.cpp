#include "ESP32ModbusRTUSlave.h"

ModbusRTUSlave::ModbusRTUSlave(uint8_t dePin, uint8_t rePin, uint8_t rxPin, uint8_t txPin)  :
    _dePin(dePin), 
    _rePin(rePin), 
    _serial(&Serial2), 
    _rxPin(rxPin), 
    _txPin(txPin) {}

void ModbusRTUSlave::begin(uint32_t baudRate, SerialConfig config, uint8_t slaveID, uint16_t numRegisters) {
    _config = config;
    _slaveID = slaveID;
    _numRegisters = numRegisters;
    _registers = new uint16_t[numRegisters]();
    _charTimeout = (11 * 1000000) / baudRate; // 1 caractere = 11 bits

    pinMode(_dePin, OUTPUT);
    pinMode(_rePin, OUTPUT);

    _serial->begin(baudRate, config, _rxPin, _txPin);
    enableReceive();
}

void ModbusRTUSlave::setRegister(uint16_t reg, uint16_t value) {
    if (reg < _numRegisters) {
        _registers[reg] = value;
    }
}

uint16_t ModbusRTUSlave::getRegister(uint16_t reg) {
    return (reg < _numRegisters) ? _registers[reg] : 0;
}

void ModbusRTUSlave::handleRequest() {
    if (_serial->available()) {
        uint8_t frame[256];
        uint8_t length = 0;

        while (_serial->available()) {
            frame[length++] = _serial->read();
            delayMicroseconds(_charTimeout); // Garante fluxo contínuo
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
                default:
                    sendException(frame[1], 0x01); // Função inválida
            }
        }
    }
}

void ModbusRTUSlave::enableTransmit() {
    digitalWrite(_dePin, HIGH);
    digitalWrite(_rePin, HIGH);
}

void ModbusRTUSlave::enableReceive() {
    digitalWrite(_dePin, LOW);
    digitalWrite(_rePin, LOW);
}

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
        sendException(0x03, 0x02); // Endereço inválido
    }
}

void ModbusRTUSlave::processFunction06(uint8_t *frame, uint8_t length) {
    uint16_t address = (frame[2] << 8) | frame[3];
    uint16_t value = (frame[4] << 8) | frame[5];

    if (address < _numRegisters) {
        _registers[address] = value;

        enableTransmit();
        _serial->write(frame, length); // Ecoa a requisição como resposta
        _serial->flush();
        enableReceive();
    } else {
        sendException(0x06, 0x02); // Endereço inválido
    }
}