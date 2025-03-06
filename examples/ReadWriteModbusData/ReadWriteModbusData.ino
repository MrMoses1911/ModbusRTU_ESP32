#include <ModbusRTU_ESP32.h>

// DE, RE, RX, TX pins
ModbusRTU_ESP32 modbus(GPIO_NUM_47, GPIO_NUM_48, GPIO_NUM_33, GPIO_NUM_34);

uint16_t rValue1, rValue2, rValue3;
uint64_t lastMillis;

void setup() {
    // Serial interface to show the data coming from the master
    Serial.begin(115200);

    // Communication Baudrate, bits configuration, Device ID, number of registers to be allocated
    modbus.beginRTU(9600, SERIAL_8E1, 1, 6);
}

void loop() {
    // OBLIGATORY IN THE LOOP FUNCTION: Updates de modbus registers and handles the master's requests
    modbus.handleRequest();

    // *You can freely read or write to the registers using the modbus functions*

    // Set fixed values to be sent to the master
    // modbus.setRegister(Register Address, Value)
    modbus.setRegister(0, 254);
    modbus.setRegister(1, 5696);
    modbus.setRegister(2, 32487);

    // Read the values being sent by the master
    // modbus.getRegister(Register Address)
    rValue1 = modbus.getRegister(3);
    rValue2 = modbus.getRegister(4);
    rValue3 = modbus.getRegister(5);

    // Delay to show the data coming from the master on the serial monitor
    if (millis() > lastMillis + 2000) {
        Serial.printf("Incoming Data:\nRegister 3 = %d\nRegister 4 = %d\nRegister 5 = %d\n\n", rValue1, rValue2, rValue3);
        lastMillis = millis();  
    }
}