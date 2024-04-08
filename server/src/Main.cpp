#include <iostream>
#include <vector>
#include <signal.h>

#define USE_LOGGER_MACROS
#include "Logger.h"

#include "Server.h"

volatile sig_atomic_t stop;

void quitHandle(int signum) {
    stop = 1;
}

int main()
{
    std::cout << "Starting server and logger..." << std::endl;

    signal(SIGINT, quitHandle);

    auto& logger = Logger::Get();
    logger.AddOutput(LogLevel::Debug | LogLevel::Info, Logger::StandardOutput);
    logger.AddOutput(LogLevel::Warning | LogLevel::Error, Logger::StandardErrorOutput);

    auto& s = Server::Get();
    s.Start();

    while (!stop)
    {
        s.HandleMessages();
    }

    s.Stop();

    return 0;
}