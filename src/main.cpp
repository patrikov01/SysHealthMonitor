#include "healthmonitor.h"

#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>
#include <string>

namespace
{
    // The signal handler only changes this flag. The monitor loop does the actual cleanup.
    std::atomic<bool> stopRequested(false);

    void handleSignal(int)
    {
        stopRequested.store(true);
    }

    void printUsage(const char* program)
    {
        std::cout << "Usage: " << program
                  << " [interval-seconds] [log-file] [critical-samples]\n";
    }
}

int main(int argc, char* argv[])
{
    MonitorConfig config;

    try
    {
        if (argc > 1)
        {
            const int seconds = std::stoi(argv[1]);
            if (seconds <= 0)
            {
                throw std::invalid_argument("interval");
            }
            config.interval = std::chrono::seconds(seconds);
        }

        if (argc > 2)
        {
            config.logFile = argv[2];
        }

        if (argc > 3)
        {
            const int samples = std::stoi(argv[3]);
            if (samples <= 0)
            {
                throw std::invalid_argument("critical samples");
            }
            config.criticalSamples = static_cast<unsigned int>(samples);
        }

        if (argc > 4)
        {
            printUsage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception&)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    HealthMonitor monitor(config);
    return monitor.run(stopRequested);
}
