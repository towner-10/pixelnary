#pragma once
#include <deque>
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "Message.h"

class ClientConnection
{
public:
    ClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message>& msgInQueue);

public:
    bool IsConnected() const;
    void SendMessage(const Message& message);

    void Disconnect();

public:
    unsigned int Id()
    {
        return m_id;
    }

    asio::ip::address IpAddress()
    {
        return m_socket.remote_endpoint().address();
    }

private:
    void AsyncSendHeader();
    void AsyncSendPayload();

    void AsyncReceiveHeader();
    void AsyncReceivePayload();


private:
    asio::ip::tcp::socket m_socket;
    asio::io_context& m_context;
    unsigned int m_id;

    Message m_incomingMsg;
    
    // We might need these to be locking...
    std::deque<Message>& m_incomingMessageQueue;
    std::deque<Message> m_outgoingMessageQueue;
};