#pragma once

#include <utility>
#include <vector>
#include <string>
#include "MessageTypes.h"
#include "Connection.h"

class Room
{
public:
    // Constructors
    Room();

    // Methods
    void AddClient(std::shared_ptr<ClientConnection> client);
    void MoveClient(ClientConnection &client, Room &room);
    void RemoveClient(ClientConnection &client);
    void RemoveClient(std::vector<std::pair<std::shared_ptr<ClientConnection>, bool>>::iterator it);
    void Broadcast(const Message &message);
    void BroadcastExcept(const Message &message, unsigned int id);
    
    // Canvas methods    
    void AddDrawCommand(MessageTypes::DrawCommand drawCommand);
    void AddDrawCommands(std::vector<MessageTypes::DrawCommand> drawCommands);
    void ClearCanvas();
    void SendCanvas();
    void SendCanvas(unsigned int id);

    bool CheckWord(const std::string &word, unsigned int id);
    void NewRound();

    // Getters
    ClientConnection *GetClient(unsigned int id);
    ClientConnection *GetLastClient();
    unsigned int CurrentDrawer() const
    {
        return m_currentDrawer;
    }

    const std::string &Word() const
    {
        return m_word;
    }

    // Iterators
    std::vector<std::pair<std::shared_ptr<ClientConnection>, bool>>::iterator begin()
    {
        return m_clients.begin();
    }

    std::vector<std::pair<std::shared_ptr<ClientConnection>, bool>>::iterator end()
    {
        return m_clients.end();
    }

    // Move operations
    Room(Room &&other) noexcept
        : m_clients(std::move(other.m_clients)),
          m_word(std::move(other.m_word)),
          m_drawCommands(std::move(other.m_drawCommands))
    {
    }

    Room &operator=(Room &&other) noexcept
    {
        m_clients = std::move(other.m_clients);
        m_drawCommands = std::move(other.m_drawCommands);
        m_word = std::move(other.m_word);
        return *this;
    }

private:
    unsigned int m_currentDrawer = 0;
    std::string m_word;
    std::vector<MessageTypes::DrawCommand> m_drawCommands;
    std::vector<std::pair<std::shared_ptr<ClientConnection>, bool>> m_clients;

    void PromoteDrawer(unsigned int id);
    void GenerateWord();

    std::vector<std::string> m_words = {
        "car", "tree", "house", "sun", "book", "dog", "cat", "chair", "ball", "boat", "clock", "hat", "cup", "key", "chair", "flower", "apple", "moon", "star", "bird"
    };
};