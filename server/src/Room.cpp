#define USE_LOGGER_MACROS

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "Logger.h"
#include "Room.h"

Room::Room()
{
    // Seed random number generator
    srand(time(nullptr));
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
    auto it = m_clients.begin();
    while (it != m_clients.end())
    {
        if (it->first.get() == &client)
        {
            RemoveClient(it);
            return;
        }
        it++;
    }
}

void Room::RemoveClient(std::vector<std::pair<std::unique_ptr<ClientConnection>, bool>>::iterator it)
{
    unsigned int id = it->first->Id();
    m_clients.erase(it);

    // Promote another client to drawer if the drawer left
    if (id == m_currentDrawer)
    {
        if (!m_clients.empty())
            PromoteDrawer(m_clients.front().first->Id());
        else
            m_currentDrawer = 0;
    }

    INFO("[Room] Removed client " + std::to_string(id) + " from room");

    // Determine if game is over
    if (!m_clients.empty())
    {
        bool allCorrect = true;
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
        }

        if (allCorrect)
        {
            INFO("[Room] All remaining clients guessed the word correctly");
            NewRound();
        }
    }
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

void Room::ClearCanvas()
{
    m_drawCommands.clear();
}

void Room::SendCanvas()
{
    Message message(MessageTypes::PacketType::CanvasPacket);
    message.Head().client_id = 0;
    message.Head().room = 0;

    MessageTypes::CanvasPacket canvasPacket;
    canvasPacket.commands = m_drawCommands;

    message.PushCanvasPacket(canvasPacket);

    Broadcast(message);
}

void Room::SendCanvas(unsigned int id)
{
    Message message(MessageTypes::PacketType::CanvasPacket);
    message.Head().client_id = 0;
    message.Head().room = 0;

    MessageTypes::CanvasPacket canvasPacket;
    canvasPacket.commands = m_drawCommands;

    message.PushCanvasPacket(canvasPacket);

    GetClient(id)->SendMessage(message);
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
    std::string copy = word;
    std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);

    bool correct = m_word == copy;

    for (auto &client : m_clients)
    {
        if (client.first->Id() == id)
        {
            client.second = correct;

            if (correct)
            {
                Message message(MessageTypes::PacketType::CorrectGuess);
                message.Head().client_id = id;
                message.Head().room = 0;
                message.Head().payload_size = 0;
                Broadcast(message);
            }
            else
            {
                Message message(MessageTypes::PacketType::GuessPacket);
                message.Head().client_id = id;
                message.Head().room = 0;
                message.Head().payload_size = sizeof(MessageTypes::GuessPacket);
                message.PushGuessPacket({word});
                Broadcast(message);
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

    GenerateWord();
    ClearCanvas();

    for (auto &client : m_clients)
    {
        client.second = false;
    }

    if (!m_clients.empty())
    {
        // Get the next drawer to the right of the current drawer
        auto it = m_clients.begin();
        while (it->first->Id() != m_currentDrawer)
        {
            it++;
        }
        it++;
        if (it == m_clients.end())
        {
            it = m_clients.begin();
        }

        PromoteDrawer(it->first->Id());
        return;
    }

    INFO("[Room] No clients in room to promote to drawer");
}

void Room::PromoteDrawer(unsigned int id)
{
    if (m_clients.empty())
    {
        INFO("[Room] No clients in room");
        return;
    }

    ClientConnection *client = GetClient(id);

    if (client == nullptr)
    {
        INFO("[Room] Client not found");
        return;
    }

    m_currentDrawer = client->Id();
    INFO("[Room] Promoted client " + std::to_string(m_currentDrawer) + " to drawer");

    for (auto &client : m_clients)
    {
        Message message(MessageTypes::PacketType::SetDrawer);
        message.Head().client_id = client.first->Id();
        message.Head().room = 0;

        bool isDrawer = client.first->Id() == m_currentDrawer;
        std::string word = "";

        if (isDrawer)
            word = Word();

        message.PushDrawerPacket({isDrawer, word});
        client.first->SendMessage(message);

        if (isDrawer)
            SendCanvas(client.first->Id());
    }
}

void Room::GenerateWord()
{
    // Randomize word using the m_words array
    m_word = m_words[rand() % m_words.size()];
}
