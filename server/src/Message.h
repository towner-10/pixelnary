#pragma once
#include <exception>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include "MessageTypes.h"

class Message
{
public:

    friend class ClientConnection;

    Message(MessageTypes::PacketType type)
        : m_header(type, 0)
    {

    }

    size_t Size() const
    {
        return sizeof(MessageTypes::Header) + m_payload.size();
    }

    size_t PayloadSize()
    {
        return m_payload.size();
    }

    MessageTypes::Header& Head()
    {
        return m_header;
    }

    void ResizePayload(size_t size)
    {
        m_payload.resize(size);
    }

    void* PayloadData()
    {
        return m_payload.data();
    }

    const std::vector<uint8_t>& Payload()
    {
        return m_payload;
    }

public:

    void PushCanvasPacket(const MessageTypes::CanvasPacket& canvasPacket);
    void PushGuessPacket(const MessageTypes::GuessPacket& guessPacket);
    void PushDrawerPacket(const MessageTypes::DrawerPacket& drawerPacket);
    void PushDrawCommand(const MessageTypes::DrawCommand& drawCommand);

    MessageTypes::CanvasPacket PopCanvasPacket();
    MessageTypes::GuessPacket PopGuessPacket();

public:
    std::string AsString() const;

    friend std::ostream& operator<<(std::ostream& os, const Message& msg)
    {
        os << msg.AsString();
        return os;
    }

public:
    class MessageTypeMismatchExcpetion : public std::exception
    {
    public:
        MessageTypeMismatchExcpetion(const std::string& what)
        {
            m_what = "[MessageTypeMismatchException] " + what;
        }
        
        const char* what() const noexcept override
        {
            return m_what.c_str();
        }

    private:
        std::string m_what;
    };

private:
    Message()
        : m_header(MessageTypes::PacketType::Null, 0)
    {

    }

private:
    void Push(const void* data, size_t size);
    bool Pop(void* buffer, size_t bufferSz, size_t size);

private:
    MessageTypes::Header m_header;
    std::vector<uint8_t> m_payload;
};