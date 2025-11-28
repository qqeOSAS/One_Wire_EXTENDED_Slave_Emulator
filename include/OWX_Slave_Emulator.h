#pragma once
#include <Arduino.h>
#include <OneWireHub.h>
#include <OneWireItem.h>
#include <functional>

// Packet command definitions

#define OW_LOW_CMD_SEND_VARIABLE_ 0x01  // відправка змінної зі слейва
#define OW_CMD_INT8        0x0F  // payload: 1 byte (int8_t)
#define OW_CMD_UINT8       0x0C  // payload: 1 byte (uint8_t)
#define OW_CMD_INT16       0x0E  // payload: 2 bytes (int16_t, LSB first)
#define OW_CMD_UINT16      0x0D  // payload: 2 bytes (uint16_t, LSB first)
#define OW_CMD_UINT32      0x12  // payload: 4 bytes (uint32_t, LSB first)
#define OW_CMD_INT32       0x10  // payload: 4 bytes (int32_t,)
#define OW_CMD_FLOAT32     0x11  
#define OW_CMD_CHAR8       0x13  // payload: 1 byte (char)
#define OW_CMD_STRUCT      0x14  

#define OW_READ_SCRATCHPAD     0x20  
#define OW_CMD_ACK         0x30 


#define OW_SCRATCHPAD_SIZE 9  // розмір scratchpad в байтах
#define OW_MAX_PAYLOAD 32  // достатньо для типових payload; підлаштуй за потреби


// [CMD SEND_VARIABLE | CMD_variable | LEN | PAYLOAD... | CRC8 ]
//        1                   1         1       N           1

class Emulator : public OneWireItem
{
public:
    enum DataType : uint8_t {
        DATA_NONE = 0,
        DATA_INT8,
        DATA_UINT8,
        DATA_INT16,
        DATA_UINT16,
        DATA_INT32,
        DATA_UINT32,
        DATA_FLOAT32,
        DATA_STRUCT
    };
private:
    uint8_t scratchpadLen;
    uint8_t scratchpad[9];
    uint8_t rawBufferLen;
    uint8_t lastCommand;

    int8_t int8_value;
    uint8_t uint8_value;
    int16_t int16_value;
    uint16_t uint16_value;
    int32_t int32_value;
    uint32_t uint32_value;
    float float_value;

    // нові поля
    volatile bool dataAvailable;
    volatile DataType lastDataType;

    std::function<bool(uint8_t)> customHandler; // callback для кастомних команд
    void read_variable_payload(OneWireHub *hub);
    

public:
    Emulator(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4,
             uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void duty(OneWireHub *hub);
    void writeScratchpad_byte(uint8_t* data, uint8_t len, uint8_t addr = 0);

    // --- нові функції ---
    uint8_t getLastCommand() const;
    void setCustomHandler(std::function<bool(uint8_t)> handler);

    // availability API
    bool available() const;
    DataType availableType() const;
    void clearAvailable();

    bool process_specific_payload_Command(uint8_t cmd_data_type, const uint8_t *payload, uint8_t len, OneWireHub *hub);
    void send_packet(uint8_t cmd, uint8_t *data, uint8_t len, OneWireHub *hub);


    int8_t getInt8() const;
    uint8_t getUInt8() const;
    int16_t getInt16() const;
    uint16_t getUInt16() const;
    int32_t getInt32() const;
    uint32_t getUInt32() const;
    float getFloat() const;

  
};


//
