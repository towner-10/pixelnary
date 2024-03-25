#define USE_LOGGER_MACROS
#include <utility>
#include "Logger.h"
#include "Server.h"

Server::Server(int port)
    : m_port(port), m_connectionAcceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{

}

void Server::Start()
{
    AsyncWaitForConnection();

    m_serverThread = std::thread([this]() {
        m_context.run();
    });

    INFO("[Server] server started on port " + std::to_string(m_port));
}

void Server::Stop()
{
    m_context.stop();

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }
    INFO("[Server] server stopped");
}

void Server::AsyncWaitForConnection()
{
    m_connectionAcceptor.async_accept([this](asio::error_code error, asio::ip::tcp::socket socket) {
        if (error)
        {
            ERROR_FL("[Server] async_accept failed with error: " + error.message());
            AsyncWaitForConnection();
            return;
        }

        m_connections.emplace_back(std::make_unique<ClientConnection>(
            m_context, std::move(socket), m_numConnections++, m_incomingMessageQueue
        ));

        OnConnect(*m_connections.back());

        AsyncWaitForConnection();
    });
}

void Server::SendMessage(ClientConnection& client, const Message& message)
{
    if (!client.IsConnected())
    {
        OnDisconnect(client);
        return;
    }

    client.SendMessage(message);
}

void Server::OnConnect(ClientConnection& client)
{
    INFO("[Server] " + client.IpAddress().to_string() + " joined the game with id "
            + std::to_string(client.Id()));
}

void Server::OnDisconnect(ClientConnection& client)
{
    // Other disconnect stuff goes here

    // Remove it from the vector
    // see https://en.cppreference.com/w/cpp/container/vector/erase
    for (auto it = m_connections.begin(); it != m_connections.end();)
    {
        if (it->get() == &client)
        {
            it = m_connections.erase(it);
            LOG_DEBUG("client erased");
            break;
        }
    }
}

void Server::OnMessage(ClientConnection& client)
{
}

void Server::HandleMessages()
{

}
