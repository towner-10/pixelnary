#include <utility>
#define USE_LOGGER_MACROS
#include "Logger.h"
#include "Connection.h"
#include "Server.h"

ClientConnection::ClientConnection(asio::io_context &context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message> &msgInQueue)
    : m_context(context), m_socket(std::move(socket)), m_id(id), m_incomingMessageQueue(msgInQueue)
{
    AsyncReceiveHeader();
}

bool ClientConnection::IsConnected() const
{
    return m_socket.is_open();
}

void ClientConnection::SendMessage(const Message &message)
{
    // Do this on the server thread
    asio::post(m_context, [this, message]() {
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
        m_socket.close();
    }

    Server::Get().OnDisconnect(*this);
}

void ClientConnection::AsyncSendHeader()
{
    asio::async_write(m_socket, asio::buffer(&m_outgoingMessageQueue.front().Head(), sizeof(MessageTypes::Header)), [this](asio::error_code error, size_t)
                      {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(m_id) + "] an error occurred when sending a message header.");
            Disconnect();
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
        } });
}

void ClientConnection::AsyncSendPayload()
{
    asio::async_write(m_socket, asio::buffer(m_outgoingMessageQueue.front().PayloadData(), m_outgoingMessageQueue.front().PayloadSize()), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(m_id) + "] an error occurred when sending a message payload.");
            Disconnect();
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
    asio::async_read(m_socket, asio::buffer(&m_incomingMsg.Head(), sizeof(MessageTypes::Header)), [this](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection_" + std::to_string(m_id) + "] client disconnected.");
            }
            else
            {
                ERROR("[Connection_" + std::to_string(m_id) + "] an error occurred when reading a message header. With error message: " + error.message());
            }

            Disconnect();
            return;
        }

        // Print the header
        LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] Header: { room: " + std::to_string(m_incomingMsg.Head().room) +
                  ", client_id: " + std::to_string(m_incomingMsg.Head().client_id) +
                  ", packet_type: " + std::to_string(static_cast<uint8_t>(m_incomingMsg.Head().packet_type)) +
                  ", payload_size: " + std::to_string(m_incomingMsg.Head().payload_size) + " }");

        m_incomingMsg.ResizePayload(m_incomingMsg.Head().payload_size);
        LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] Received header");
        AsyncReceivePayload(); // Nesting lambdas appears to be harder than I thought
    });
}

void ClientConnection::AsyncReceivePayload()
{
    asio::async_read(m_socket, asio::buffer(m_incomingMsg.PayloadData(), m_incomingMsg.Payload().size()), [this](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection_" + std::to_string(m_id) + "] client disconnected.");
            }
            else
            {
                ERROR("[Connection_" + std::to_string(m_id) + "] an error occurred when reading a message payload. With error message: " + error.message());
            }

            Disconnect();
            return;
        }

        m_incomingMessageQueue.emplace_back(std::move(m_incomingMsg));
        m_incomingMsg = Message(); // reset the object
        LOG_DEBUG("[Connection_" + std::to_string(m_id) + "] Received payload");
        
        AsyncReceiveHeader();
    });
}