#include <ModbusRTU_MAX485.h>

ModbusRTU_MAX485 modbus(GPIO_NUM_47, GPIO_NUM_48, GPIO_NUM_33, GPIO_NUM_34);

uint16_t rValue1, rValue2, rValue3;
uint64_t lastMillis;

void setup() {
    Serial.begin(115200);

    modbus.beginRTU(9600, SERIAL_8E1, 1, 3);
}

void loop() {
    modbus.handleRequest();

    modbus.setRegister(0, 254);
    modbus.setRegister(1, 5696);
    modbus.setRegister(2, 32487);

    rValue1 = modbus.getRegister(0);
    rValue2 = modbus.getRegister(1);
    rValue3 = modbus.getRegister(2);

    if (millis() > lastMillis + 2000) {
        Serial.printf("Register 1: %d\nRegister 2: %d\nRegister 3: %d\n\n", rValue1, rValue2, rValue3);
        lastMillis = millis();
    }
}