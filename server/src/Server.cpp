#define USE_LOGGER_MACROS
#include <utility>
#include "Logger.h"
#include "Server.h"
#include "MessageTypes.h"

Server::Server(unsigned int port)
    : m_port(port), m_connectionAcceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    m_rooms = std::vector<Room>();
}

void Server::Start()
{
    AsyncWaitForConnection();

    m_serverThread = std::thread([this]() { m_context.run(); });
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

unsigned int Server::CreateRoom()
{
    m_rooms.emplace_back();
    return m_rooms.size() - 1;
}

bool Server::MoveFromWaitingRoomToRoom(ClientConnection &client, unsigned int room)
{
    if (room >= m_rooms.size())
    {
        ERROR_FL("[Server] Room " + std::to_string(room) + " does not exist");
        return false;
    }

    for (auto it = m_waitingRoom.begin(); it != m_waitingRoom.end();)
    {
        if (it->first.get() == &client)
        {
            INFO("[Server] Moving connection " + std::to_string(client.Id()) + " to room " + std::to_string(room));
            m_waitingRoom.MoveClient(*it->first.get(), m_rooms[room]);
            return true;
        }
    }

    ERROR_FL("[Server] Connection " + std::to_string(client.Id()) + " was not found in the waiting room");
    return false;
}

void Server::AsyncWaitForConnection()
{
    m_connectionAcceptor.async_accept([this](asio::error_code error, asio::ip::tcp::socket socket)
                                      {
        if (error)
        {
            ERROR_FL("[Server] async_accept failed with error: " + error.message());
            AsyncWaitForConnection();
            return;
        }

        m_waitingRoom.AddClient(std::make_unique<ClientConnection>(
            m_context, std::move(socket), m_numConnections++, m_incomingMessageQueue
        ));

        OnConnect(*m_waitingRoom.GetLastClient());

        // BTW... this is not recursion. This function is async which means
        // it returns virtually instantly. So by doing this, we are not growing
        // the stack frame but rather
        AsyncWaitForConnection(); });
}

void Server::SendMessage(ClientConnection &client, const Message &message)
{
    if (!client.IsConnected())
    {
        OnDisconnect(client);
        return;
    }

    client.SendMessage(message);
}

void Server::OnConnect(ClientConnection &client)
{
    INFO("[Server] " + client.IpAddress().to_string() + " joined the game with id " + std::to_string(client.Id()));

    // Send their ID
    Message message(MessageTypes::PacketType::SetClientId);
    message.Head().client_id = client.Id();
    message.Head().room = 0;
    message.Head().payload_size = 0;
    SendMessage(client, message);
}

void Server::OnDisconnect(ClientConnection &client)
{
    INFO("[Server] Connection " + std::to_string(client.Id()) + " left the game");

    auto it = m_waitingRoom.begin();
    while (it != m_waitingRoom.end())
    {
        if (it->first.get() == &client)
        {
            INFO("[Server] Connection " + std::to_string(client.Id()) + " was in the waiting room");
            m_waitingRoom.RemoveClient(it);
            return;
        }
        else
        {
            it++;
        }
    }

    for (int i = 0; i < m_rooms.size(); i++)
    {
        Room &room = m_rooms[i];
        auto it = room.begin();
        while (it != room.end())
        {
            if (it->first.get() == &client)
            {
                room.RemoveClient(it);
                return;
            }
            else
            {
                it++;
            }
        }
    }

    ERROR("[Server] Connection " + std::to_string(client.Id()) + " was not found in the waiting room or any room");
}

void Server::HandleMessages()
{
    while (!m_incomingMessageQueue.empty())
    {
        Message message = m_incomingMessageQueue.front();

        switch (message.Head().packet_type)
        {
        case MessageTypes::PacketType::JoinRoom:
        {
            ClientConnection *client = m_waitingRoom.GetClient(message.Head().client_id);
            if (client != nullptr)
            {
                bool success = MoveFromWaitingRoomToRoom(*client, message.Head().room);

                if (!success)
                {
                    ERROR("[Server] Failed to move client " + std::to_string(message.Head().client_id) + " to room " + std::to_string(message.Head().room));
                    break;
                }

                Message response(MessageTypes::PacketType::SetDrawer);
                response.Head().client_id = message.Head().client_id;
                response.Head().room = message.Head().room;
                response.Head().payload_size = sizeof(MessageTypes::DrawerPacket);

                bool isDrawer = m_rooms[message.Head().room].CurrentDrawer() == message.Head().client_id;
                
                std::string word = "";
                if (isDrawer)
                    word = m_rooms[message.Head().room].Word();

                response.PushDrawerPacket({m_rooms[message.Head().room].CurrentDrawer() == message.Head().client_id, word});
                SendMessage(*m_rooms[message.Head().room].GetClient(message.Head().client_id), response);
                m_rooms[message.Head().room].SendCanvas(message.Head().client_id);

                break;
            }
            ERROR("[Server] Client " + std::to_string(message.Head().client_id) + " was not found in the waiting room");
            break;
        }
        case MessageTypes::PacketType::CreateRoom:
        {
            INFO("[Server] Received CreateRoom message from client " + std::to_string(message.Head().client_id));
            unsigned int newRoom = CreateRoom();
            Message response(MessageTypes::PacketType::SetRoomId);
            response.Head().client_id = message.Head().client_id;
            response.Head().room = newRoom;
            response.Head().payload_size = 0;

            ClientConnection *client = m_waitingRoom.GetClient(message.Head().client_id);

            if (client == nullptr)
            {
                ERROR("[Server] Client " + std::to_string(message.Head().client_id) + " was not found in the waiting room");
                break;
            }

            SendMessage(*m_waitingRoom.GetClient(message.Head().client_id), response);
            break;
        }
        case MessageTypes::PacketType::DrawCommand:
        {
            INFO("[Server] Received DrawCommand message from client " + std::to_string(message.Head().client_id));
            if (message.Head().room >= m_rooms.size())
            {
                ERROR("[Server] Room " + std::to_string(message.Head().room) + " does not exist");
                break;
            }
            std::vector<MessageTypes::DrawCommand> commands = message.PopCanvasPacket().commands;

            if (commands.size() != 1)
            {
                ERROR("[Server] Received DrawCommand message with " + std::to_string(commands.size()) + " commands");
                break;
            }

            m_rooms[message.Head().room].AddDrawCommands(commands);
            message.PushDrawCommand(commands[0]);
            m_rooms[message.Head().room].BroadcastExcept(message, message.Head().client_id);
            break;
        }
        case MessageTypes::PacketType::GuessPacket:
        {
            INFO("[Server] Received GuessPacket message from client " + std::to_string(message.Head().client_id));

            if (message.Head().room >= m_rooms.size())
            {
                ERROR("[Server] Room " + std::to_string(message.Head().room) + " does not exist");
                break;
            }

            Room &room = m_rooms[message.Head().room];

            if (room.CurrentDrawer() == message.Head().client_id)
            {
                ERROR("[Server] Client " + std::to_string(message.Head().client_id) + " is the drawer and cannot guess");
                break;
            }

            room.CheckWord(message.PopGuessPacket().guess, message.Head().client_id);
            break;
        }
        case MessageTypes::PacketType::CanvasPacket:
        {
            INFO("[Server] Received CanvasCommand message from client " + std::to_string(message.Head().client_id));
            if (message.Head().room >= m_rooms.size())
            {
                ERROR("[Server] Room " + std::to_string(message.Head().room) + " does not exist");
                break;
            }
            m_rooms[message.Head().room].ClearCanvas();
            m_rooms[message.Head().room].SendCanvas();
            break;
        }
        default:
            ERROR("[Server] Received unknown message of type " + std::to_string(static_cast<uint8_t>(message.Head().packet_type)) + " from client " + std::to_string(message.Head().client_id));
            break;
        }

        m_incomingMessageQueue.pop_front();
    }
}
