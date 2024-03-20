#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

class Message
{
public:

    struct Header
    {
        Header(uint32_t id, uint32_t payloadSize)
            : id(id), payloadSize(payloadSize)
        {}
        uint32_t id;
        uint32_t payloadSize;
    };

public:

    Message(uint32_t id)
        : m_header(id, 0)
    {

    }

    size_t Size() const
    {
        return sizeof(Header) + m_payload.size();
    }

    size_t PayloadSize()
    {
        return m_payload.size();
    }

    Header Head()
    {
        return m_header;
    }

    const std::vector<uint8_t>& Payload()
    {
        return m_payload;
    }

public:

    /**
     * This can get dangerous. It might be a good idea to use some sort of templated functions
     * or at least some for common types (i.e. PushInt(), PushString(), etc.).
     * A particular case of interest is that of strings. You must be espically carful if you choose not to
     * send a null terminator, which is likely for this project.
     * 
     * That said, this will work for now.
    */
    void Push(const void* data, size_t size);
    bool Pop(void* buffer, size_t bufferSz, size_t size);

public:
    std::string AsString() const;

    friend std::ostream& operator<<(std::ostream& os, const Message& msg)
    {
        os << msg.AsString();
        return os;
    }

private:
    Header m_header;
    std::vector<uint8_t> m_payload;
};

class Server
{
public:
    Server();
};