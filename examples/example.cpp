/************************************************ *************
  
		OWX Slave Emulator FULL DEMO

		Demonstrates the use of OWX_Slave_Emulator library to create
		a virtual extended 1-Wire device that can receive various data types
	  	from a 1-Wire master.

	tested on: ESP8266 (NodeMCU, Wemos D1 Mini Pro)
	by qqeOSAS(2025)
	

 *************************************************************




*/

#include <Arduino.h>
#include <OneWire.h>
#include <OneWireHub.h>
#include <OWX_Slave_Emulator.h>

#define ONEWIRE_PIN 2

// Create global 1-Wire hub on pin 2
OneWireHub hub(ONEWIRE_PIN);

// Create one virtual 1-Wire device (Emulator)
Emulator slaveEmu(
    0x01,0x02,0x03,0x04,0x05,0x06,0x07
);

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println(F("\n=== OWX Slave Emulator FULL DEMO ==="));

    // Attach emulator to the hub
    hub.attach(slaveEmu);

    Serial.println(F("Slave attached. Waiting for master commands...\n"));
}

void loop() {
    hub.poll();  // MUST be called often!

    // Check if master sent new payload
    if (slaveEmu.available()) {
        Emulator::DataType type = slaveEmu.availableType();

        Serial.println(F("[DATA RECEIVED]"));

        switch(type) {

            case Emulator::DATA_INT8:
                Serial.print(F(" INT8: "));
                Serial.println(slaveEmu.getInt8());
                break;

            case Emulator::DATA_UINT8:
                Serial.print(F(" UINT8: "));
                Serial.println(slaveEmu.getUInt8());
                break;

            case Emulator::DATA_INT16:
                Serial.print(F(" INT16: "));
                Serial.println(slaveEmu.getInt16());
                break;

            case Emulator::DATA_UINT16:
                Serial.print(F(" UINT16: "));
                Serial.println(slaveEmu.getUInt16());
                break;

            case Emulator::DATA_INT32:
                Serial.print(F(" INT32: "));
                Serial.println(slaveEmu.getInt32());
                break;

            case Emulator::DATA_UINT32:
                Serial.print(F(" UINT32: "));
                Serial.println(slaveEmu.getUInt32());
                break;

            case Emulator::DATA_FLOAT32:
                Serial.print(F(" FLOAT32: "));
                Serial.println(slaveEmu.getFloat(), 4);
                break;

            case Emulator::DATA_STRUCT:
                Serial.println(F(" STRUCT received"));
                // тут ти можеш розпарсити структуру
                break;

            default:
                Serial.println(F(" Unknown type"));
                break;
        }

        slaveEmu.clearAvailable();
    }

    // Every 1s update scratchpad dynamically
    static unsigned long tmr = 0;
    unsigned long now = millis();
    if (now - tmr >= 1000) {
        tmr = now;

        // Example: write random value into scratchpad[3]
        uint8_t randomVal = random(0, 255);
        slaveEmu.writeScratchpad_byte(&randomVal, 1, 3);

        Serial.print(F("Scratchpad[3] updated with: "));
        Serial.println(randomVal);
    }
}



/***************************************************************
  
      OPTIONAL: ADVANCED FEATURES

      - Custom command handler


****************************************************************/

bool customCommandHandler(uint8_t cmd) {
    Serial.print(F("Custom CMD handled: 0x"));
    Serial.println(cmd, HEX);
    return true;  // command processed
}

// EXAMPLE STRUCT FORMAT (you can adjust):
// struct MyStruct { int16_t temperature; uint16_t humidity; float voltage; };
void parseStructExample(const uint8_t* data, uint8_t len) {
    if(len < 8) return;

    int16_t temperature;
    uint16_t humidity;
    float voltage;

    memcpy(&temperature, data, 2);
    memcpy(&humidity, data + 2, 2);
    memcpy(&voltage, data + 4, 4);

    Serial.print(F("STRUCT → Temp="));
    Serial.print(temperature);
    Serial.print(F(" Hum="));
    Serial.print(humidity);
    Serial.print(F(" V="));
    Serial.println(voltage);
}
