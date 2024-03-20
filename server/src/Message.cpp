#include <cstring>
#include <sstream>
#include "Server.h"

void Message::Push(const void* data, size_t size)
{
    size_t oldSize = m_payload.size();

    m_payload.resize(oldSize + size);

    memcpy(m_payload.data() + oldSize, data, size);

    m_header.payloadSize = m_payload.size();
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

    m_header.payloadSize = m_payload.size();

    return true;
}

std::string Message::AsString() const
{
    std::stringstream ss;

    constexpr size_t BUF_SZ = 16;
    char buf[BUF_SZ];

    ss << "************************ Message ************************\n";

    snprintf(buf, BUF_SZ, "%04d", m_header.id);
    ss << "ID: " << buf << "                               ";
    snprintf(buf, BUF_SZ, "%04d", m_header.payloadSize);
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