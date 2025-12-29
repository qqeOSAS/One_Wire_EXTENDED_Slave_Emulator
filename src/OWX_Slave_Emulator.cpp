#include <OWX_Slave_Emulator.h>
#include <Arduino.h>

// Constructor initializes device ROM, scratchpad state and internal buffers
Emulator::Emulator(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4,
                   uint8_t ID5, uint8_t ID6, uint8_t ID7)
    : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    // Scratchpad is not pre-filled; default length = 9 bytes
    scratchpadLen = 9;

    lastCommand = 0x00;
    rawBufferLen = 0;

    int32_value = 0;
    float_value = 0.0f;

    customHandler = nullptr;
    dataAvailable = false;
    lastDataType = Emulator::DATA_NONE;
}

// Local CRC8 calculation used for packet verification
uint8_t crc8_local(const uint8_t *data, size_t len, uint8_t crc_init = 0) {
    uint8_t crc = crc_init;
    while (len--) {
        uint8_t in = *data++;
        for (uint8_t i = 0; i < 8; ++i) {
            uint8_t mix = (crc ^ in) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;   // Dallas/Maxim CRC polynomial
            in >>= 1;
        }
    }
    return crc;
}

// Sends a generic packet: CMD + LEN + PAYLOAD + CRC
void Emulator::send_packet(uint8_t cmd, uint8_t* data, uint8_t len, OneWireHub *hub){
    // Prepare CRC source buffer
    uint8_t crc_buf[2 + len];
    crc_buf[0] = cmd;
    crc_buf[1] = len;

    for(uint8_t i = 0; i < len; i++)
        crc_buf[2 + i] = data[i];

    uint8_t crc = crc8_local(crc_buf, 2 + len);

    // OneWire custom low-level header
    uint8_t header = OW_LOW_CMD_SEND_VARIABLE_;
    hub->send(&header, 1);

    // Send CMD and length
    hub->send(&cmd, 1);
    hub->send(&len, 1);  // Length is always one byte

    // Send payload
    for(uint8_t i = 0; i < len; i++)
        hub->send(&data[i], 1);

    // Send CRC
    hub->send(&crc, 1);
}

// Writes multiple bytes into the scratchpad at a specific offset
void Emulator::writeScratchpad_byte(uint8_t* data, uint8_t len, uint8_t addr) {
    // Bounds check: do not overflow scratchpad
    if(len > OW_SCRATCHPAD_SIZE) return;

    uint8_t max_aviable_len = OW_SCRATCHPAD_SIZE - addr;
    if(len > max_aviable_len) return;

    // Write byte-by-byte
    for(uint8_t i = 0; i < len; i++)
        scratchpad[addr + i] = data[i];
}


// Writes an int8_t value into the scratchpad at a specific offset
void Emulator::writeScratchpad_int8(int8_t value, uint8_t addr) {
    writeScratchpad_byte(reinterpret_cast<uint8_t*>(&value), sizeof(int8_t), addr);
}

// Writes a uint8_t value into the scratchpad at a specific offset
void Emulator::writeScratchpad_uint8(uint8_t value, uint8_t addr) {
    writeScratchpad_byte(&value, sizeof(uint8_t), addr);
}

// Writes an int16_t value into the scratchpad at a specific offset
void Emulator::writeScratchpad_int16(int16_t value, uint8_t addr) {
    writeScratchpad_byte(reinterpret_cast<uint8_t*>(&value), sizeof(int16_t), addr);
}

// Writes a uint16_t value into the scratchpad at a specific offset
void Emulator::writeScratchpad_uint16(uint16_t value, uint8_t addr) {
    writeScratchpad_byte(reinterpret_cast<uint8_t*>(&value), sizeof(uint16_t), addr);
}

// Writes an int32_t value into the scratchpad at a specific offset
void Emulator::writeScratchpad_int32(int32_t value, uint8_t addr) {
    writeScratchpad_byte(reinterpret_cast<uint8_t*>(&value), sizeof(int32_t), addr);
}

// Writes a uint32_t value into the scratchpad at a specific offset
void Emulator::writeScratchpad_uint32(uint32_t value, uint8_t addr) {
    writeScratchpad_byte(reinterpret_cast<uint8_t*>(&value), sizeof(uint32_t), addr);
}

// Writes a float value into the scratchpad at a specific offset
void Emulator::writeScratchpad_float(float value, uint8_t addr) {
    writeScratchpad_byte(reinterpret_cast<uint8_t*>(&value), sizeof(float), addr);
}

// Reads a structured payload: CMD + LEN + PAYLOAD + CRC
void Emulator::read_variable_payload(OneWireHub *hub){
    uint8_t packet_header[2];
    
    // Read CMD + LEN
    if (hub->recv(packet_header, 2)) 
        return;

    uint8_t payload_len = packet_header[1];
    if (payload_len == 0) return;

    // Validate length against max allowed
    if (payload_len > OW_MAX_PAYLOAD) {
        hub->raiseDeviceError(packet_header[0]);
        return;
    }

    // Read payload into a fixed-size buffer
    uint8_t payload_buf[OW_MAX_PAYLOAD];
    if (hub->recv(payload_buf, payload_len)) {
        hub->raiseDeviceError(packet_header[0]);
        return;
    }

    // Read CRC byte
    uint8_t recv_crc;
    if (hub->recv(&recv_crc, 1)){
        hub->raiseDeviceError(packet_header[0]);
        return; 
    }

    // Recalculate CRC from received data
    uint8_t calc_crc = crc8_local(packet_header, 2);
    calc_crc = crc8_local(payload_buf, payload_len, calc_crc);

    if (calc_crc != recv_crc) {
        // CRC mismatch → corruption detected
        hub->raiseDeviceError(packet_header[0]);
        return;
    }

    // Store last command ID
    lastCommand = packet_header[0];

    // Dispatch to command-specific handler
    bool handled = process_specific_payload_Command(packet_header[0], payload_buf, payload_len, hub);

    // Acknowledge correctly handled commands
    if(handled){
        uint8_t ack = OW_CMD_ACK;
        hub->send(&ack, 1);
    }
}
void parse_handler_command(OneWireHub *hub){
    uint8_t handler_command;

    if(hub->recv(&handler_command, 1)) return;

    if(customHandler){
        bool result = customHandler(handler_command);  // ✅ ОЦЕ ПРАВИЛЬНО

        if(result){
            uint8_t ack = OW_CMD_ACK;
            hub->send(&ack, 1);
        }
    }
    
}
// Main low-level dispatcher for incoming OneWire commands
void Emulator::duty(OneWireHub *hub){
    uint8_t low_cmd;

    // Receive low-level 1-byte command
    if(hub->recv(&low_cmd, 1)) return;

    switch(low_cmd){
        case OW_READ_SCRATCHPAD:
            // Send all scratchpad bytes
            Serial.println(F("OW_READ_SCRATCHPAD command received"));
            for(uint8_t i = 0; i < scratchpadLen; i++)
                hub->send(&scratchpad[i], 1);
        break;
        
        case OW_LOW_CMD_SEND_VARIABLE_:
            // Higher-level packet incoming
            read_variable_payload(hub);
            break;
        case OW_HANDLER_COMMAND: 
            // Custom handler command
            parse_handler_command(hub);
            break;
            

        default:
            // Unknown low-level command (ignored)
            break;
    }
}

// Handles payload and decodes values according to data type
bool Emulator::process_specific_payload_Command(
    uint8_t cmd_data_type, const uint8_t *payload, uint8_t len, OneWireHub *hub)
{
    bool handled = false;

    switch(cmd_data_type){
        // --- 8-bit signed ---
        case OW_CMD_INT8:
            int8_value = (int8_t)payload[0];
            lastDataType = DATA_INT8;
            dataAvailable = true;
            handled = true;
            break;

        // --- 8-bit unsigned ---
        case OW_CMD_UINT8:
            uint8_value = payload[0];
            lastDataType = DATA_UINT8;
            dataAvailable = true;
            handled = true;
            break;

        // --- 16-bit signed ---
        case OW_CMD_INT16:
            memcpy(&int16_value, payload, 2);  // Little-endian safe on ESP
            lastDataType = DATA_INT16;
            dataAvailable = true;
            handled = true;
            break;

        // --- 16-bit unsigned ---
        case OW_CMD_UINT16:
            memcpy(&uint16_value, payload, 2);
            lastDataType = DATA_UINT16;
            dataAvailable = true;
            handled = true;
            break;

        // --- 32-bit signed ---
        case OW_CMD_INT32:
            if(len != 4) { 
                hub->raiseDeviceError(cmd_data_type); 
                return false; 
            }
            memcpy(&int32_value, payload, 4);
            lastDataType = DATA_INT32;
            dataAvailable = true;
            handled = true;
            break;

        // --- 32-bit unsigned ---
        case OW_CMD_UINT32:
            if(len != 4) { 
                hub->raiseDeviceError(cmd_data_type); 
                return false; 
            }
            memcpy(&uint32_value, payload, 4);
            lastDataType = DATA_UINT32;
            dataAvailable = true;
            handled = true;
            break;

        // --- 32-bit float ---
        case OW_CMD_FLOAT32:
            if(len != 4) { 
                hub->raiseDeviceError(cmd_data_type); 
                return false; 
            }
            memcpy(&float_value, payload, 4);
            lastDataType = DATA_FLOAT32;
            dataAvailable = true;
            handled = true;
            break;

        // --- Struct parsing not implemented yet ---
        case OW_CMD_STRUCT:
            // Reserved for user-defined structured payloads
            break;

        // --- Fallback for custom user handlers ---
        default:
            break;
    }
    return handled;

}
// --- Getters for decoded values ---
int8_t Emulator::getInt8() const { return int8_value; }
int16_t Emulator::getInt16() const { return int16_value; }
uint8_t Emulator::getUInt8() const { return uint8_value; }
uint16_t Emulator::getUInt16() const { return uint16_value; }
int32_t Emulator::getInt32() const { return int32_value; }
uint32_t Emulator::getUInt32() const { return uint32_value; }
float Emulator::getFloat() const { return float_value; }

// --- API for checking if data was received ---
bool Emulator::available() const { return dataAvailable; }
Emulator::DataType Emulator::availableType() const { return lastDataType; }
void Emulator::clearAvailable() { dataAvailable = false; lastDataType = DATA_NONE; }

// --- Get last received command code ---
uint8_t Emulator::getLastCommand() const { return lastCommand; }

// --- Install user-defined command handler ---
void Emulator::setCustomHandler(std::function<bool(uint8_t)> handler) { 
    customHandler = handler; 
}
