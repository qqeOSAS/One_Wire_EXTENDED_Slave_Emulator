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
    slaveEmu.setCustomHandler(myCustomHandler);

 
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
                break;

            default:
                Serial.println(F(" Unknown type"));
                break;
        }

        slaveEmu.clearAvailable();
    }

  
}



/***************************************************************
  
      OPTIONAL: ADVANCED FEATURES

      - Custom command handler


****************************************************************/

// Custom handler function for a single condition
bool myCustomHandler(uint8_t cmd) {
    Serial.print(F("[CUSTOM HANDLER] Command received: 0x"));
    if (cmd == OW_REQUEST_UPDATE) {  
        uint8_t random_uint8 = random(0,255);// Example custom command
        uint16_t random_uint16 = random(0,65535);
        float random_float = random(0,100) + random(0,99) / 100;
        slaveEmu.writeScratchpad_uint8(random_uint8, 0); // Write value 42 at scratchpad[0]
        slaveEmu.writeScratchpad_uint16(random_uint16, 1); // Write value 12345 at scratchpad[1-2]
        slaveEmu.writeScratchpad_float(random_float, 3); // Write float at scratchpad[3-6]
        // Serial.printf("Updated data uint8 %d, uint16 %d, float %.2f\n", random_uint8, random_uint16, random_float);
                      
        return true;  // Indicate the command was handled
    }
    return false;  // Command not recognized
}
//wwewewefgitsdsdsdsdsd