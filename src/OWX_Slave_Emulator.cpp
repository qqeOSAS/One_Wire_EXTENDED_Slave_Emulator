#include <OWX_Slave_Emulator.h>
#include <Arduino.h>

Emulator::Emulator(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4,
                   uint8_t ID5, uint8_t ID6, uint8_t ID7)
    : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    //memset(scratchpad, 0, sizeof(scratchpad));
    lastCommand = 0x00;
    scratchpadLen = 4;  // на початку scratchpad порожній

    rawBufferLen = 0;
    int32_value = 0;
    float_value = 0.0f;
    customHandler = nullptr;
    dataAvailable = false;
    lastDataType = Emulator::DATA_NONE;
}

uint8_t crc8_local(const uint8_t *data, size_t len, uint8_t crc_init = 0) {
    uint8_t crc = crc_init;
    while (len--) {
        uint8_t in = *data++;
        for (uint8_t i = 0; i < 8; ++i) {
            uint8_t mix = (crc ^ in) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            in >>= 1;
        }
    }
    return crc;
}

void Emulator::send_packet(uint8_t cmd, uint8_t* data, uint8_t len, OneWireHub *hub){
    // ---------------- CRC preparation ----------------
    uint8_t crc_buf[2 + len];
    crc_buf[0] = cmd;
    crc_buf[1] = len;

    for(uint8_t i = 0; i < len; i++)
        crc_buf[2 + i] = data[i];

    uint8_t crc = crc8_local(crc_buf, 2 + len);

    // ---------------- Actual transmission ----------------
    uint8_t header = OW_LOW_CMD_SEND_VARIABLE_;
    hub->send(&header, 1);

    hub->send(&cmd, 1);
    hub->send(&len, 1);  // FIX: only 1 byte

    for(uint8_t i = 0; i < len; i++)
        hub->send(&data[i], 1);
    

    hub->send(&crc, 1);
}

//set scratchpad bytes in specified address
void Emulator::writeScratchpad_byte(uint8_t* data, uint8_t len, uint8_t addr) {

    if(len >OW_SCRATCHPAD_SIZE) return;

    uint8_t max_aviable_len = OW_SCRATCHPAD_SIZE - addr; // aviable bytes from addr to end
    if( len > max_aviable_len) return;

    for(uint8_t i = 0; i < len; i++)
        scratchpad[addr + i] = data[i];
    
}

void Emulator::read_variable_payload(OneWireHub *hub){
    uint8_t packet_header[2];
    if (hub->recv(packet_header, 2)) 
        return;

    if (packet_header[1] == 0)
        return;  

    uint8_t* payload = (uint8_t*)malloc(packet_header[1]);
        if (!payload) {
            hub->raiseDeviceError(packet_header[0]);
            return;
        }
            //if payload not received
    if (hub->recv(payload, packet_header[1])) {
        free(payload);
        hub->raiseDeviceError(packet_header[0]);
        return;
    }

    // check if crc byte received 
    uint8_t recv_crc;
    if (hub->recv(&recv_crc, 1)){
        free(payload);
        hub->raiseDeviceError(packet_header[0]);
        return; 
    }

            // comparing CRC8
    uint8_t calc_crc = crc8_local(packet_header, 2);       // CMD + LEN
    calc_crc = crc8_local(payload, packet_header[1], calc_crc);         // payload

    if (calc_crc != recv_crc) {
        free(payload);
        hub->raiseDeviceError(packet_header[0]);
        return;
    }

    // зберігаємо останню команду
    lastCommand = packet_header[0];
    
    // виклик винесеної обробки
    bool handled = process_specific_payload_Command(packet_header[0], payload, packet_header[1], hub);

    // if handled successfully, send ACK
    if(handled){
        uint8_t ack = OW_CMD_ACK;
        hub->send(&ack, 1);
    }

    free(payload);

}
// головна функція для обробки подій на шині
void Emulator::duty(OneWireHub *hub){

    uint8_t low_cmd;
    if(hub->recv(&low_cmd, 1)) return;

    Serial.printf("0x%02X\n", low_cmd);



    switch(low_cmd){
        case OW_READ_SCRATCHPAD:
            Serial.println(F("OW_READ_SCRATCHPAD command received"));
            for(uint8_t i = 0; i < scratchpadLen; i++)
                hub->send(&scratchpad[i], 1); // надсилаємо байт за байтом

        break;
        
        case OW_LOW_CMD_SEND_VARIABLE_:
            read_variable_payload(hub);
            break;
    }
}

bool Emulator::process_specific_payload_Command(uint8_t cmd_data_type, const uint8_t *payload, uint8_t len, OneWireHub *hub){
    bool handled = false;

    switch(cmd_data_type){
        case OW_CMD_INT8:
            int8_value = (int8_t)payload[0];
            lastDataType = DATA_INT8;
            dataAvailable = true;
            Serial.print(F("Received INT8: ")); Serial.println(int8_value);
            handled = true;
            break;

        case OW_CMD_INT16:
            int16_value = (int16_t)(payload[0] | (payload[1]<<8));
            lastDataType = DATA_INT16;
            dataAvailable = true;
            Serial.print(F("Received INT16: ")); Serial.println(int16_value);
            handled = true;
            break;

        case OW_CMD_UINT16:
            uint16_value = (uint16_t)(payload[0] | (payload[1]<<8));
            lastDataType = DATA_UINT16;
            dataAvailable = true;
            Serial.print(F("Received UINT16: ")); Serial.println(uint16_value);
            handled = true;
            break;

        case OW_CMD_INT32:
            int32_value = (int32_t)( (uint32_t)payload[0] | ((uint32_t)payload[1]<<8) | ((uint32_t)payload[2]<<16) | ((uint32_t)payload[3]<<24) );
            lastDataType = DATA_INT32;
            dataAvailable = true;
            Serial.print(F("Received INT32: ")); Serial.println(int32_value);
            handled = true;
            break;

        case OW_CMD_UINT32:
            if(len != 4) { hub->raiseDeviceError(cmd_data_type); return false; }
            uint32_value = (uint32_t)( (uint32_t)payload[0] | ((uint32_t)payload[1]<<8) | ((uint32_t)payload[2]<<16) | ((uint32_t)payload[3]<<24) );
            lastDataType = DATA_UINT32;
            dataAvailable = true;
            Serial.print(F("Received UINT32: ")); Serial.println(uint32_value);
            handled = true;
            break;

        case OW_CMD_FLOAT32:
            if(len != 4) { hub->raiseDeviceError(cmd_data_type); return false; }
            memcpy(&float_value, payload, 4);
            lastDataType = DATA_FLOAT32;
            dataAvailable = true;
            Serial.print(F("Received FLOAT32: ")); Serial.println(float_value, 4);
            handled = true;
            break;

        case OW_CMD_STRUCT:
    
            break;

        default:
            if(customHandler) {
                handled = customHandler(cmd_data_type);
                if(handled){
                    Serial.print(F("Custom handler processed CMD: 0x")); Serial.println(cmd_data_type, HEX);
                }
            }

            if(!handled){
                Serial.print(F("Unknown CMD: 0x")); Serial.println(cmd_data_type, HEX);
                hub->raiseDeviceError(cmd_data_type);
            }
            break;
    }

    return handled;
}


// --- Геттери ---
int8_t Emulator::getInt8() const { return int8_value; }
int16_t Emulator::getInt16() const { return int16_value; }
uint16_t Emulator::getUInt16() const { return uint16_value; }
int32_t Emulator::getInt32() const { return int32_value; }
uint32_t Emulator::getUInt32() const { return uint32_value; }
float Emulator::getFloat() const { return float_value; }

// availability API
bool Emulator::available() const { return dataAvailable; }
Emulator::DataType Emulator::availableType() const { return lastDataType; }
void Emulator::clearAvailable() { dataAvailable = false; lastDataType = DATA_NONE; }

// встановлюємо значення у scratchpad (як приклад)
//void Emulator::setValue(int value) { scratchpad[0] = value & 0xFF; }
//int Emulator::getValue() const { return scratchpad[0]; }
//const uint8_t* Emulator::getScratchpad() const { return scratchpad; }



// доступ до останнього отриманого байту / команди
uint8_t Emulator::getLastCommand() const { return lastCommand; }


// встановлення callback
void Emulator::setCustomHandler(std::function<bool(uint8_t)> handler) { customHandler = handler; }


