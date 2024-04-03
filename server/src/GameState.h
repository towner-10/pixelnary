#pragma once
#include <vector>

class GameState
{
public:
    void RegisterClient(unsigned int id)
    {
        m_players.push_back(id);
    }

private:
    std::vector<unsigned int> m_players;
    // Game state goes here
};