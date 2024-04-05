#include <cstring>
#include <sstream>
#include "Message.h"

void Message::PushCanvasPacket(const MessageTypes::CanvasPacket& canvasPacket)
{
    if (m_header.packet_type != MessageTypes::PacketType::CanvasPacket)
    {
        throw MessageTypeMismatchExcpetion("attempt to push a message type that does not match the type of the message");
    }

    Push(canvasPacket.commands.data(), canvasPacket.commands.size() * sizeof(MessageTypes::DrawCommand));
}

void Message::PushGuessPacket(const MessageTypes::GuessPacket& guessPacket)
{
    if (m_header.packet_type != MessageTypes::PacketType::GuessPacket)
    {
        throw MessageTypeMismatchExcpetion("attempt to push a message type that does not match the type of the message");
    }

    const char* guess = guessPacket.guess.c_str();
    Push(guess, strlen(guess));
}

void Message::PushDrawerPacket(const MessageTypes::DrawerPacket& drawerPacket)
{
    if (m_header.packet_type != MessageTypes::PacketType::SetDrawer)
    {
        throw MessageTypeMismatchExcpetion("attempt to push a message type that does not match the type of the message");
    }

    Push(&drawerPacket.isDrawer, sizeof(bool));
}

MessageTypes::CanvasPacket Message::PopCanvasPacket()
{
    if (m_header.packet_type != MessageTypes::PacketType::DrawCommand)
    {
        throw MessageTypeMismatchExcpetion("attempt to pop a message to a type that does not match the type of the message");
    }

    MessageTypes::CanvasPacket ret;

    const size_t numElements = m_header.payload_size / sizeof(MessageTypes::PacketType::CanvasPacket);
    ret.commands.resize(numElements);

    Pop(ret.commands.data(), m_header.payload_size, m_header.payload_size);

    return ret;
}

MessageTypes::GuessPacket Message::PopGuessPacket()
{
    if (m_header.packet_type != MessageTypes::PacketType::GuessPacket)
    {
        throw MessageTypeMismatchExcpetion("attempt to pop a message to a type that does not match the type of the message");
    }

    MessageTypes::GuessPacket ret;
    std::vector<uint8_t> buf(m_header.payload_size);
    Pop(buf.data(), buf.size(), m_header.payload_size);

    ret.guess = std::string(buf.begin(), buf.end());    
    return ret;
}

void Message::Push(const void* data, size_t size)
{
    size_t oldSize = m_payload.size();

    m_payload.resize(oldSize + size);

    memcpy(m_payload.data() + oldSize, data, size);

    m_header.payload_size = m_payload.size();
}

bool Message::Pop(void* buffer, size_t bufferSz, size_t size)
{
    // Exceptions... nah
    if (size > bufferSz)
    {
        return false;
    }

    if (size > m_payload.size())
    {
        return false;
    }

    size_t startPos = m_payload.size() - size;

    memcpy(buffer, m_payload.data() + startPos, size);

    m_payload.resize(m_payload.size() - size);

    m_header.payload_size = m_payload.size();

    return true;
}

std::string Message::AsString() const
{
    std::stringstream ss;

    constexpr size_t BUF_SZ = 16;
    char buf[BUF_SZ];

    ss << "************************ Message ************************\n";

    snprintf(buf, BUF_SZ, "%02d", (uint8_t)m_header.packet_type);
    ss << "ID: " << buf << "                                 ";
    snprintf(buf, BUF_SZ, "%04d", m_header.payload_size);
    ss << "Payload Size: " << buf << '\n';
    ss << "---------------------------------------------------------\n";

    for (int i = 0; i < m_payload.size(); ++i)
    {
        if (i % 16 == 0)
        {
            if (i > 0) ss << "\n";
            snprintf(buf, BUF_SZ, "%08x: ", i);
            ss << buf;
        }

        snprintf(buf, BUF_SZ, "%02x ", m_payload[i]);
        ss << buf;
    }

    ss << "\n*********************************************************\n";
    return ss.str();
}