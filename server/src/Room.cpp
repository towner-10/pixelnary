#define USE_LOGGER_MACROS
#include "Logger.h"
#include "Room.h"

Room::Room()
{
    NewRound();
}

void Room::AddClient(std::unique_ptr<ClientConnection> client)
{
    // Set as drawer if no clients are in the room
    bool drawer = m_clients.empty();
    m_clients.push_back(std::make_pair(std::move(client), drawer));

    if (drawer)
    {
        m_currentDrawer = m_clients.back().first->Id();
    }
}

void Room::MoveClient(ClientConnection &client, Room &room)
{
    for (auto it = m_clients.begin(); it != m_clients.end();)
    {
        if (it->first.get() == &client)
        {
            if (it->first->Id() == m_currentDrawer)
            {
                m_currentDrawer = 0;
            }

            room.AddClient(std::move(it->first));
            m_clients.erase(it);
            return;
        }
    }
}

void Room::RemoveClient(ClientConnection &client)
{
    for (auto it = m_clients.begin(); it != m_clients.end();)
    {
        if (it->first.get() == &client)
        {
            RemoveClient(it);
            return;
        }
    }
}

void Room::RemoveClient(std::vector<std::pair<std::unique_ptr<ClientConnection>, bool>>::iterator it)
{
    // Promote another client to drawer if the drawer left
    if (it->first->Id() == m_currentDrawer)
    {
        m_clients.erase(it);
        PromoteNextDrawer();
        return;
    }

    m_clients.erase(it);
}

void Room::Broadcast(const Message &message)
{
    for (auto &client : m_clients)
    {
        client.first->SendMessage(message);
    }
}

void Room::BroadcastExcept(const Message &message, unsigned int id)
{
    for (auto &client : m_clients)
    {
        if (client.first->Id() != id)
        {
            client.first->SendMessage(message);
        }
    }
}

void Room::AddDrawCommand(MessageTypes::DrawCommand drawCommand)
{
    m_drawCommands.push_back(drawCommand);
}

void Room::AddDrawCommands(std::vector<MessageTypes::DrawCommand> drawCommands)
{
    for (auto &drawCommand : drawCommands)
    {
        m_drawCommands.push_back(drawCommand);
    }
}

void Room::ClearDrawCommands()
{
    m_drawCommands.clear();
}

void Room::SendCanvas()
{
    Message message(MessageTypes::PacketType::CanvasPacket);

    MessageTypes::CanvasPacket canvasPacket;
    canvasPacket.commands.reserve(m_drawCommands.size());

    for (auto &drawCommand : m_drawCommands)
    {
        canvasPacket.commands.push_back(drawCommand);
    }

    message.PushCanvasPacket(canvasPacket);
    BroadcastExcept(message, m_currentDrawer);
}

ClientConnection *Room::GetClient(unsigned int id)
{
    for (auto &client : m_clients)
    {
        if (client.first->Id() == id)
        {
            return client.first.get();
        }
    }

    return nullptr;
}

ClientConnection *Room::GetLastClient()
{
    if (m_clients.empty())
    {
        return nullptr;
    }

    return m_clients.back().first.get();
}

bool Room::CheckWord(const std::string &word, unsigned int id)
{
    bool correct = m_word == word;

    for (auto &client : m_clients)
    {
        if (client.first->Id() == id)
        {
            client.second = correct;

            if (correct)
            {
                client.first->SendMessage(Message(MessageTypes::PacketType::CorrectGuess));
            }
            else
            {
                client.first->SendMessage(Message(MessageTypes::PacketType::IncorrectGuess));
            }

            bool allCorrect = false;

            for (auto &client : m_clients)
            {
                if (client.first->Id() == m_currentDrawer)
                {
                    continue;
                }

                if (!client.second)
                {
                    allCorrect = false;
                    break;
                }
                allCorrect = true;
            }

            if (allCorrect)
            {
                NewRound();
            }
        }
    }

    return correct;
}

void Room::NewRound()
{
    INFO("[Room] Starting new round");

    m_word = "test";
    ClearDrawCommands();

    for (auto &client : m_clients)
    {
        client.second = false;
    }

    Broadcast(Message(MessageTypes::PacketType::NewRound));
}

void Room::PromoteNextDrawer()
{
    if (m_clients.empty())
    {
        INFO("[Room] No clients in room");
        return;
    }

    m_currentDrawer = m_clients.front().first->Id();
    INFO("[Room] Promoted client " + std::to_string(m_currentDrawer) + " to drawer");

    for (auto &client : m_clients)
    {
        Message message(MessageTypes::PacketType::SetDrawer);
        message.Head().client_id = client.first->Id();
        message.Head().room = 0;
        message.Head().payload_size = sizeof(MessageTypes::DrawerPacket);
        message.PushDrawerPacket({client.first->Id() == m_currentDrawer});
        client.first->SendMessage(message);
    }
}
