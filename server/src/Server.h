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
#include "Room.h"

class Server
{

public:
    static Server& Get()
    {
        static Server instance(25565);
        return instance;
    }

public:
    void Start();
    void Stop();

    void AsyncWaitForConnection();

    void SendMessage(ClientConnection& client, const Message& message);

    void OnConnect(ClientConnection& client);
    void OnDisconnect(ClientConnection& client);

    unsigned int CreateRoom();
    void CreateRoom(unsigned int room);
    void MoveFromWaitingRoomToRoom(ClientConnection& client, unsigned int room);

    void HandleMessages();

private:
    Server(unsigned int port);

    Server(const Server&) = delete;
    Server(Server&&) = delete;

    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;

private:
    unsigned int m_port;
    unsigned int m_numConnections = 0;
    Room m_waitingRoom;
    std::vector<Room> m_rooms;

    std::deque<Message> m_incomingMessageQueue;

    asio::io_context m_context;
    asio::ip::tcp::acceptor m_connectionAcceptor;
    std::thread m_serverThread;
};