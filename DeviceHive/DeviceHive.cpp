#include "DeviceHive.h"
#include "Arduino.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define DH_SIGNATURE1   0xC5
#define DH_SIGNATURE2   0xC3
#define DH_VERSION      0x01


// default/main constructor
Message::Message(uint16_t msg_intent)
    : intent(msg_intent)
    , length(0)
{}



// main constructor
OutputMessage::OutputMessage(uint16_t intent)
    : Message(intent)
{}


// put custom binary data
void OutputMessage::put(const void *buf, unsigned int len)
{
    if (length+len <= MAX_MSG_SIZE)
    {
        memcpy(buffer+length, buf, len);
        length += len;
    }
    // else: error NO MORE SPACE
}


// put custom string
void OutputMessage::putString(const char *str, unsigned int len)
{
    if (length+sizeof(len)+len <= MAX_MSG_SIZE)
    {
        putUInt16(len);
        put(str, len);
    }
    // else: error NO MORE SPACE
}

// put NULL-terminated string
void OutputMessage::putString(const char *str)
{
    putString(str, strlen(str));
}


// put unsigned 32-bits integer (little-endian)
void OutputMessage::putUInt32(uint32_t val)
{
    if (length+sizeof(val) <= MAX_MSG_SIZE)
    {
        buffer[length++] = (val   ) & 0xFF;
        buffer[length++] = (val>>8) & 0xFF;
        buffer[length++] = (val>>16) & 0xFF;
        buffer[length++] = (val>>24) & 0xFF;
    }
    // else: error NO MORE SPACE
}


// put unsigned 16-bits integer (little-endian)
void OutputMessage::putUInt16(uint16_t val)
{
    if (length+sizeof(val) <= MAX_MSG_SIZE)
    {
        buffer[length++] = (val   ) & 0xFF;
        buffer[length++] = (val>>8) & 0xFF;
    }
    // else: error NO MORE SPACE
}


// put unsigned 8-bits integer
void OutputMessage::putUInt8(uint8_t val)
{
    if (length+sizeof(val) <= MAX_MSG_SIZE)
    {
        buffer[length++] = val;
    }
    // else: error NO MORE SPACE
}


// put signed 32-bits integer (little-endian)
void OutputMessage::putInt32(int32_t val)
{
    putUInt32(val);
}


// put signed 16-bits integer (little-endian)
void OutputMessage::putInt16(int16_t val)
{
    putUInt16(val);
}


// put signed 8-bits integer
void OutputMessage::putInt8(int8_t val)
{
    putUInt8(val);
}



// default constructor
InputMessage::InputMessage()
    : Message(0)
    , read_pos(0)
{}


// reset the "read" position
void InputMessage::reset()
{
    read_pos = 0;
}


// skip data
void InputMessage::skip(unsigned int len)
{
    if (read_pos+len <= length)
    {
        read_pos += len;
    }
    // else: error NO MORE DATA
}


// get custom binary data
void InputMessage::get(void *buf, unsigned int len)
{
    if (read_pos+len <= length)
    {
        memcpy(buf, buffer + read_pos, len);
        read_pos += len;
    }
    // else: error NO MORE DATA
}


// get NULL-terminated string, return string length, always NULL-terminated
unsigned int InputMessage::getString(char *str, unsigned int max_len)
{
    if (read_pos+sizeof(uint16_t) <= length)
    {
        const uint16_t len = getUInt16();
        if (read_pos+len <= length)
        {
            if (len < max_len)
            {
                get(str, len);
                str[len] = 0;
                return len;
            }
            else
            {
                get(str, max_len-1);
                skip(len-max_len+1);
                str[max_len-1] = 0;
                return max_len;
            }
        }
        // else: error NO MORE DATA
    }
    // else: error NO MORE DATA

    if (max_len > 0)
        str[0] = 0;
    return 0;
}


// get unsigned 32-bits integer (little-endian)
uint32_t InputMessage::getUInt32()
{
    uint32_t val = 0;

    if (read_pos+sizeof(val) <= length)
    {
        val  =            buffer[read_pos++];
        val |= ((uint32_t)buffer[read_pos++]) << 8;
        val |= ((uint32_t)buffer[read_pos++]) << 16;
        val |= ((uint32_t)buffer[read_pos++]) << 24;
    }
    // else: error NO MORE DATA

    return val;
}


// get unsigned 16-bits integer (little-endian)
uint16_t InputMessage::getUInt16()
{
    uint16_t val = 0;

    if (read_pos+sizeof(val) <= length)
    {
        val  =            buffer[read_pos++];
        val |= ((uint16_t)buffer[read_pos++]) << 8;
    }
    // else: error NO MORE DATA

    return val;
}


// get unsigned 8-bits integer
uint8_t InputMessage::getUInt8()
{
    uint8_t val = 0;

    if (read_pos+sizeof(val) <= length)
    {
        val = buffer[read_pos++];
    }
    // else: error NO MORE DATA

    return val;
}


// get signed 32-bits integer (little-endian)
int32_t InputMessage::getInt32()
{
    return getUInt32();
}


// get signed 16-bits integer (little-endian)
int16_t InputMessage::getInt16()
{
    return getUInt16();
}


// get signed 8-bits integer
int8_t InputMessage::getInt8()
{
    return getUInt8();
}



// default constructor
DeviceHive::DeviceHive()
    : stream(0)
    , rx_timeout(1000)
    , rx_state(STATE_SIGNATURE1)
    , rx_msg_len(0)
    , rx_checksum(0)
    , rx_started_at(0)
{}


// set RX timeout, milliseconds
void DeviceHive::setRxTimeout(unsigned long ms)
{
    rx_timeout = ms;
}


// prepare engine with Serial stream
void DeviceHive::begin(Stream *stream)
{
    this->stream = stream;
}


// prepare engine with Serial stream
void DeviceHive::begin(Stream &stream)
{
    begin(&stream);
}


// flush the Serial stream
void DeviceHive::end()
{
    if (stream)
    {
        stream->flush();
        stream = 0;
    }
}


// try to read message, return zero if message has been read!
int DeviceHive::read(Message *msg)
{
    return msg ? read(*msg) : 0;
}


// try to read message, return zero if message has been read!
int DeviceHive::read(Message &msg)
{
    if (!stream)
        return DH_PARSE_NO_SERIAL;

    while (stream->available() > 0)
    {
        const unsigned int b = stream->read();

        // check RX timeout
        if (rx_state != STATE_SIGNATURE1 && rx_timeout)
        {
            if ((millis()-rx_started_at) > rx_timeout)
            {
                // reset RX state machine!
                rx_state = STATE_SIGNATURE1;
                return DH_PARSE_TIMED_OUT;
            }
        }

        // update RX checksum register
        if (rx_state != STATE_SIGNATURE1)
            rx_checksum += b;
        else
            rx_checksum = b;


        switch (rx_state)
        {
            case STATE_SIGNATURE1:
                if (b == DH_SIGNATURE1)
                {
                    rx_started_at = millis();
                    msg.length = 0;

                    rx_state = STATE_SIGNATURE2;
                }
                break;

            case STATE_SIGNATURE2:
                rx_state = (b == DH_SIGNATURE2) ? STATE_VERSION : STATE_SIGNATURE1;
                break;

            case STATE_VERSION:
                rx_state = (b == DH_VERSION) ? STATE_FLAGS : STATE_SIGNATURE1;
                break;

            case STATE_FLAGS: // flags are ignored
                rx_state = STATE_LENGTH1;
                break;

            case STATE_LENGTH1:
                rx_msg_len = b; // low byte
                rx_state = STATE_LENGTH2;
                break;

            case STATE_LENGTH2:
                rx_msg_len |= b << 8; // high byte
                if (rx_msg_len > MAX_MSG_SIZE)
                {
                    rx_state = STATE_SIGNATURE1;
                    return DH_PARSE_MESSAGE_TOO_BIG;
                }
                else
                    rx_state = STATE_INTENT1;
                break;

            case STATE_INTENT1:
                msg.intent = b; // low byte
                rx_state = STATE_INTENT2;
                break;

            case STATE_INTENT2:
                msg.intent |= b << 8; // high byte
                rx_state = rx_msg_len ? STATE_PAYLOAD : STATE_CHECKSUM;
                break;

            case STATE_PAYLOAD:
                if (msg.length+1 <= MAX_MSG_SIZE)
                    msg.buffer[msg.length++] = b;
                if (--rx_msg_len == 0)
                    rx_state = STATE_CHECKSUM;
                break;

            case STATE_CHECKSUM:
                rx_state = STATE_SIGNATURE1;
                if ((rx_checksum&0xFF) != 0xFF)
                    return DH_PARSE_BAD_CHECKSUM;
                else
                    return DH_PARSE_OK;
        }
    }

    return DH_PARSE_INCOMPLETE;
}


// write custom message
void DeviceHive::write(const Message *msg)
{
    if (msg)
        write(*msg);
}


// write custom message
void DeviceHive::write(const Message &msg)
{
    if (!stream)
        return;

    stream->write(DH_SIGNATURE1);
    stream->write(DH_SIGNATURE2);
    stream->write(DH_VERSION);
    stream->write((uint8_t)0x00);

    unsigned int tx_checksum = DH_SIGNATURE1
        + DH_SIGNATURE2 + DH_VERSION + 0x00;

    { // length, little endian
        const unsigned int a = (msg.length     ) & 0xFF;
        const unsigned int b = (msg.length >> 8) & 0xFF;

        stream->write(a);
        stream->write(b);
        tx_checksum += (a + b);
    }

    { // intent, little endian
        const unsigned int a = (msg.intent     ) & 0xFF;
        const unsigned int b = (msg.intent >> 8) & 0xFF;

        stream->write(a);
        stream->write(b);
        tx_checksum += (a + b);
    }

    // message body
    if (0 < msg.length)
    {
        stream->write(msg.buffer, msg.length);
        for (unsigned int i = 0; i < msg.length; ++i)
            tx_checksum += msg.buffer[i];
    }

    // checksum
    stream->write(0xFF - (tx_checksum&0xFF));
}


// write registration response (JSON format)
void DeviceHive::writeRegistrationResponse(const char *data)
{
    OutputMessage msg(INTENT_REGISTRATION_RESPONSE_JSON);
    msg.putString(data);
    write(msg);
}


// write command result
void DeviceHive::writeCommandResult(uint32_t cmd_id, const char *status, const char *result)
{
    OutputMessage msg(INTENT_COMMAND_RESULT);
    msg.putUInt32(cmd_id);
    msg.putString(status);
    msg.putString(result);
    write(msg);
}


// global DeviceHive engine instance
DeviceHive DH;
