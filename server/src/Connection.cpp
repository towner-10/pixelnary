#include <utility>
#include "Connection.h"

ClientConnection::ClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message>& msgInQueue)
    : m_context(context), m_socket(std::move(socket)), m_id(id), m_incomingMessageQueue(msgInQueue)
{
}

bool ClientConnection::IsConnected() const
{
    return m_socket.is_open();
}

void ClientConnection::SendMessage(const Message& message)
{

}

void ClientConnection::Disconnect()
{
        
}