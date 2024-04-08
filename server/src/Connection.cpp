#include <utility>
#define USE_LOGGER_MACROS
#include "Logger.h"
#include "Connection.h"
#include "Server.h"

ClientConnection::ClientConnection(asio::io_context &context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message> &msgInQueue)
    : m_context(context), m_socket(std::move(socket)), m_id(id), m_incomingMessageQueue(msgInQueue)
{
}

void ClientConnection::Start()
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
    asio::post(m_context, [self = shared_from_this(), message]() {
        bool isEmpty = self->m_outgoingMessageQueue.empty();
        self->m_outgoingMessageQueue.push_back(message);
        // Only if we aren't already sending messages at the moment
        if (isEmpty)
        {
            self->AsyncSendHeader();
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
    asio::async_write(m_socket, asio::buffer(&m_outgoingMessageQueue.front().Head(), sizeof(MessageTypes::Header)), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when sending a message header.");
            self->Disconnect();
            return;            
        }

        if (self->m_outgoingMessageQueue.front().PayloadSize() > 0)
        {
            LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] header sent");
            self->AsyncSendPayload();
            return;
        }

        self->m_outgoingMessageQueue.pop_front();

        if (!self->m_outgoingMessageQueue.empty())
        {
            self->AsyncSendHeader();
        }
    });
}

void ClientConnection::AsyncSendPayload()
{
    asio::async_write(m_socket, asio::buffer(m_outgoingMessageQueue.front().PayloadData(), m_outgoingMessageQueue.front().PayloadSize()), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when sending a message payload.");
            self->Disconnect();
            return;            
        }

        self->m_outgoingMessageQueue.pop_front();
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] payload sent");

        if (!self->m_outgoingMessageQueue.empty())
        {
            self->AsyncSendHeader();
        }
    });
}

void ClientConnection::AsyncReceiveHeader()
{
    asio::async_read(m_socket, asio::buffer(&m_incomingMsg.Head(), sizeof(MessageTypes::Header)), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection_" + std::to_string(self->Id()) + "] client disconnected.");
            }
            else
            {
                ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when reading a message header. With error message: " + error.message());
            }

            self->Disconnect();
            return;
        }
        else if (self->m_incomingMsg.Head().packet_type == MessageTypes::PacketType::Null)
        {
            ERROR("[Connection_" + std::to_string(self->Id()) + "] received a NULL message.");
            self->Disconnect();
            return;
        }

        // Print the header
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] Header: { room: " + std::to_string(self->m_incomingMsg.Head().room) +
                  ", client_id: " + std::to_string(self->m_incomingMsg.Head().client_id) +
                  ", packet_type: " + std::to_string(static_cast<uint8_t>(self->m_incomingMsg.Head().packet_type)) +
                  ", payload_size: " + std::to_string(self->m_incomingMsg.Head().payload_size) + " }");

        self->m_incomingMsg.ResizePayload(self->m_incomingMsg.Head().payload_size);
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] Received header");
        self->AsyncReceivePayload(); // Nesting lambdas appears to be harder than I thought
    });
}

void ClientConnection::AsyncReceivePayload()
{
    asio::async_read(m_socket, asio::buffer(m_incomingMsg.PayloadData(), m_incomingMsg.Payload().size()), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection_" + std::to_string(self->Id()) + "] client disconnected.");
            }
            else
            {
                ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when reading a message payload. With error message: " + error.message());
            }

            self->Disconnect();
            return;
        }

        self->m_incomingMessageQueue.emplace_back(std::move(self->m_incomingMsg));
        self->m_incomingMsg = Message(); // reset the object
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] Received payload");
        
        self->AsyncReceiveHeader();
    });
}