#ifndef ESP32ModbusRTUSlave_h
#define ESP32ModbusRTUSlave_h

#include "Arduino.h"
#include "HardwareSerial.h"

class ModbusRTUSlave {
public:
    ModbusRTUSlave(uint8_t dePin, uint8_t rePin, uint8_t rxPin, uint8_t txPin);

    void begin(uint32_t baudRate, SerialConfig config, uint8_t slaveID, uint16_t numRegisters);

    void setRegister(uint16_t reg, uint16_t value);
    uint16_t getRegister(uint16_t reg);

    void handleRequest();

private:
    uint8_t _dePin, _rePin, _slaveID, _rxPin, _txPin;
    uint16_t *_registers;
    uint16_t _numRegisters;
    HardwareSerial *_serial;
    SerialConfig _config;
    uint32_t _charTimeout; // Tempo para 1,5 caracteres

    void enableTransmit();
    void enableReceive();
    uint16_t calculateCRC(uint8_t *buffer, uint8_t length);
    void sendException(uint8_t functionCode, uint8_t exceptionCode);
    void processFunction03(uint8_t *frame, uint8_t length);
    void processFunction06(uint8_t *frame, uint8_t length);
};

#endif