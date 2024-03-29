#include <iostream>
#include <vector>

#define USE_LOGGER_MACROS
#include "Logger.h"

#include "Server.h"

int main()
{
    std::cout << "Starting server and logger..." << std::endl;

    auto &logger = Logger::Get();
    logger.AddOutput(LogLevel::Debug | LogLevel::Info, Logger::StandardOutput);
    logger.AddOutput(LogLevel::Warning | LogLevel::Error, Logger::StandardErrorOutput);

    Server s(25565);

    s.Start();

    while (true)
    {
        s.HandleMessages();
    }

    return 0;
}