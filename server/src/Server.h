#pragma once
#include <memory>
#include <vector>
#include <deque>
#include <thread>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "Message.h"
#include "Connection.h"

class Server
{
public:
    Server(int port);
    Server(const Server&) = delete;
    Server(Server&&) = delete;

    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;

public:

    void Start();
    void Stop();

    void AsyncWaitForConnection();

    void Temp();
    void SendMessage(ClientConnection& client, const Message& message);

    void OnConnect(ClientConnection& client);
    void OnDisconnect(ClientConnection& client);
    void OnMessage(ClientConnection& client);

    void HandleMessages();
private:
    int m_port;
    unsigned int m_numConnections = 0;
    std::vector<std::unique_ptr<ClientConnection>> m_connections;

    std::deque<Message> m_incomingMessageQueue;

    asio::io_context m_context;
    asio::ip::tcp::acceptor m_connectionAcceptor;
    std::thread m_serverThread;
};