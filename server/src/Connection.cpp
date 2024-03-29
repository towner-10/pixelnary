#include <utility>
#define USE_LOGGER_MACROS
#include "Logger.h"
#include "Connection.h"

ClientConnection::ClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message>& msgInQueue)
    : m_context(context), m_socket(std::move(socket)), m_id(id), m_incomingMessageQueue(msgInQueue)
{
    AsyncReceiveHeader();
}

bool ClientConnection::IsConnected() const
{
    return m_socket.is_open();
}

void ClientConnection::SendMessage(const Message& message)
{
    // Do this on the server thread
    asio::post(m_context, [this, message](){
        bool isEmpty = m_outgoingMessageQueue.empty();
        m_outgoingMessageQueue.push_back(message);
        // Only if we aren't already sending messages at the moment
        if (isEmpty)
        {
            AsyncSendHeader();
        }
    });
}

void ClientConnection::Disconnect()
{
    if (m_socket.is_open())
    {
        // Give it to the server thread so it can finish what it's working on
        asio::post(m_context, [this]() {
            m_socket.close();
        });
    }       
}

void ClientConnection::AsyncSendHeader()
{
    asio::async_write(m_socket, asio::buffer(&m_outgoingMessageQueue.front().Head(), sizeof(Message::Header)), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(m_id) + "] an error occured when sending a message header.");
            m_socket.close();
            return;            
        }

        if (m_outgoingMessageQueue.front().PayloadSize() > 0)
        {
            LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] header sent");
            AsyncSendPayload();
            return;
        }

        m_outgoingMessageQueue.pop_front();

        if (!m_outgoingMessageQueue.empty())
        {
            AsyncSendHeader();
        }        
    });
}

void ClientConnection::AsyncSendPayload()
{
    asio::async_write(m_socket, asio::buffer(m_outgoingMessageQueue.front().PayloadData(), m_outgoingMessageQueue.front().PayloadSize()), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(m_id) + "] an error occured when sending a message payload.");
            m_socket.close();
            return;            
        }

        m_outgoingMessageQueue.pop_front();
        LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] payload sent");


        if (!m_outgoingMessageQueue.empty())
        {
            AsyncSendHeader();
        }        
    });
}

void ClientConnection::AsyncReceiveHeader()
{
    asio::async_read(m_socket, asio::buffer(&m_incomingMsg.Head(), sizeof(Message::Header)), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(m_id) + "] an error occured when reading a message header.");
            m_socket.close();
            return;
        }

        // I don't know if trying to read 0 over the network will break anything
        // but just in case, only try to read a body if it exists
        if (m_incomingMsg.Head().payloadSize > 0)
        {
            m_incomingMsg.ResizePayload(m_incomingMsg.Head().payloadSize);
            LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] Received header");
            AsyncReceivePayload(); // Nesting lambdas appeares to be harder than I thought
            return;
        }

        m_incomingMessageQueue.emplace_back(std::move(m_incomingMsg));
    });
}

void ClientConnection::AsyncReceivePayload()
{
    asio::async_read(m_socket, asio::buffer(m_incomingMsg.PayloadData(), m_incomingMsg.Payload().size()), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(m_id) + "] an error occured when reading a message payload.");
            m_socket.close();
            return;
        }

        m_incomingMessageQueue.emplace_back(std::move(m_incomingMsg));
        LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] Received payload");


        AsyncReceiveHeader();
    });
}