#ifndef __DEVICE_HIVE_H__
#define __DEVICE_HIVE_H__

#include <inttypes.h>
#include "Stream.h"

#if !defined(MAX_MSG_SIZE)
#   define MAX_MSG_SIZE 256
#endif // MAX_MSG_SIZE

// basic message
class Message
{
public:
    explicit Message(uint16_t intent = 0);

public:
    uint16_t intent;                // message type identifier
    uint8_t  buffer[MAX_MSG_SIZE];  // payload buffer
    uint16_t length;                // actual payload length
};


enum Intent
{
    INTENT_REGISTRATION_REQUEST         = 0,
    //INTENT_REGISTRATION_RESPONSE_BIN    = 1,
    INTENT_REGISTRATION_RESPONSE_JSON   = 3,
    INTENT_COMMAND_RESULT               = 2,
    INTENT_USER_DEFINED                 = 256
};


// message + formatting
class OutputMessage:
    public Message
{
public:
    explicit OutputMessage(uint16_t intent);

public:
    void put(const void *buf, unsigned int len);

    void putString(const char *str, unsigned int len);
    void putString(const char *str);
    void putUInt32(uint32_t val);
    void putUInt16(uint16_t val);
    void putUInt8(uint8_t val);
    void putInt32(int32_t val);
    void putInt16(int16_t val);
    void putInt8(int8_t val);

    // Arduino friendly names
    void putULong(unsigned long val) { putUInt32(val); }
    void putUShort(unsigned short val) { putUInt16(val); }
    void putByte(byte val) { putUInt8(val); }
    void putLong(long val) { putInt32(val); }
    void putShort(short val) { putInt16(val); }
    void putChar(char val) { putUInt8(val); }
};


// message + parsing
class InputMessage:
    public Message
{
public:
    InputMessage();
    void reset();

public:
    void skip(unsigned int len);
    void get(void *buf, unsigned int len);

    unsigned int getString(char *str, unsigned int max_len);
    uint32_t getUInt32();
    uint16_t getUInt16();
    uint8_t getUInt8();
    int32_t getInt32();
    int16_t getInt16();
    int8_t getInt8();

    // Arduino friendly names
    unsigned long getULong() { return getUInt32(); }
    unsigned short getUShort() { return getUInt16(); }
    byte getByte() { return getUInt8(); }
    long getLong() { return getInt32(); }
    short getShort() { return getInt16(); }
    char getChar() { return getInt8(); }

private:
    uint16_t read_pos;      // current "read" position
};


// message parsing result
enum ParseResult
{
    DH_PARSE_OK,
    DH_PARSE_MESSAGE_TOO_BIG,
    DH_PARSE_BAD_CHECKSUM,
    DH_PARSE_INCOMPLETE,
    DH_PARSE_TIMED_OUT,
    DH_PARSE_NO_SERIAL
};


// engine: read/write messages from/to serial stream
class DeviceHive
{
public:
    DeviceHive();

    void setRxTimeout(unsigned long ms);

    void begin(Stream *stream);
    void begin(Stream &stream);
    void end();

public:

    int read(Message *msg);
    int read(Message &msg);

public:
    void write(const Message *msg);
    void write(const Message &msg);

    void writeRegistrationResponse(const char *data);
    void writeCommandResult(uint32_t cmd_id,
        const char *status, const char *result);

private:
    Stream *stream;             // Serial stream
    unsigned int rx_timeout;    // RX timeout, milliseconds

    enum ParserState            // possible parser states
    {
        STATE_SIGNATURE1,
        STATE_SIGNATURE2,
        STATE_VERSION,
        STATE_FLAGS,
        STATE_LENGTH1,
        STATE_LENGTH2,
        STATE_INTENT1,
        STATE_INTENT2,
        STATE_PAYLOAD,
        STATE_CHECKSUM
    };

    ParserState   rx_state;         // RX parser state
    uint16_t      rx_msg_len;       // expected message payload length
    unsigned int  rx_checksum;      // RX checksum register
    unsigned long rx_started_at;    // RX message start time, milliseconds
};


extern DeviceHive DH; // global DeviceHive engine instance

#endif // __DEVICE_HIVE_H__
