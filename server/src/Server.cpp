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

        // BTW... this is not recursion. This function is async which means
        // it returns virtually instantly. So by doing this, we are not growing
        // the stack frame but rather
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

    Temp();
}

void Server::OnDisconnect(ClientConnection& client)
{
    INFO("[Server] " + std::to_string(client.Id()) + " left the game");
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

// TODO: call me
void Server::OnMessage(ClientConnection& client)
{
}

void Server::HandleMessages()
{
    while (!m_incomingMessageQueue.empty())
    {
        std::cout << m_incomingMessageQueue.front();
        m_incomingMessageQueue.pop_front();
    }
}


void Server::Temp()
{
    Message msg('A');
    const char* greeting = "the quick brown fox jumps over the lazy dog";
    msg.Push(greeting, strlen(greeting));

    std::cout << msg;

    SendMessage(*m_connections.front().get(), msg);
    LOG_DEBUG("[Server] message sent");
}
