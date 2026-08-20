#include "healthmonitor.h"
#include "systemstats.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <utility>

HealthMonitor::HealthMonitor(MonitorConfig otherConfig)
    : config(std::move(otherConfig)), logger(config.logFile)
{
}

int HealthMonitor::run(const std::atomic<bool>& stopRequested)
{
    if (!logger.isOpen())
    {
        std::cerr << "Could not open log file: " << config.logFile << '\n';
        return 1;
    }

    // CPU load is calculated from the difference between two /proc/stat readings.
    std::optional<CpuTimes> previous = SystemStats::readCpuTimes();
    if (!previous)
    {
        std::cerr << "Could not read /proc/stat\n";
        return 1;
    }

    unsigned int criticalCount = 0;
    bool criticalState = false;

    while (!stopRequested.load())
    {
        const auto wakeTime = std::chrono::steady_clock::now() + config.interval;
        // Short sleeps let SIGINT and SIGTERM stop the program without waiting for the full interval.
        while (!stopRequested.load() && std::chrono::steady_clock::now() < wakeTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (stopRequested.load())
        {
            break;
        }

        const std::optional<CpuTimes> current = SystemStats::readCpuTimes();
        if (!current)
        {
            std::cerr << "Could not read /proc/stat\n";
            continue;
        }

        HealthSample sample;
        sample.cpuUsagePercent = SystemStats::calculateCpuUsage(*previous, *current);
        sample.cpuTemperatureC = SystemStats::readCpuTemperature();
        sample.gpu = SystemStats::readGpuStats();
        previous = current;

        logger.write(sample);
        std::cout << describe(sample) << '\n';

        // One normal sample breaks the consecutive critical-sample sequence.
        criticalCount = isCritical(sample) ? criticalCount + 1 : 0;
        const bool isNowCritical = criticalCount >= config.criticalSamples;
        // Notify only when entering the critical state, not on every later sample.
        if (isNowCritical && !criticalState)
        {
            sendNotification(sample);
        }
        criticalState = isNowCritical;
    }

    std::cout << "Monitor stopped.\n";
    return 0;
}

bool HealthMonitor::isCritical(const HealthSample& sample) const
{
    const bool hotCpu = sample.cpuTemperatureC && 
                        *sample.cpuTemperatureC > config.maximumCpuTemperatureC;
    return hotCpu || sample.cpuUsagePercent > config.maximumCpuUsagePercent;
}

void HealthMonitor::sendNotification(const HealthSample& sample) const
{
    const std::string message = describe(sample);
    const std::string command = "notify-send \"SysHealth ALERT\" \"" + message + "\"";
    std::system(command.c_str());
}

std::string HealthMonitor::describe(const HealthSample& sample)
{
    std::ostringstream text;
    text << std::fixed << std::setprecision(1)
         << "CPU " << sample.cpuUsagePercent << "%";

    if (sample.cpuTemperatureC)
    {
        text << ", CPU temp " << *sample.cpuTemperatureC << " C";
    }
    else
    {
        text << ", CPU temp unavailable";
    }

    if (sample.gpu)
    {
        text << ", GPU " << sample.gpu->utilizationPercent << "%"
             << ", GPU temp " << sample.gpu->temperatureC << " C"
             << ", GPU memory " << sample.gpu->memoryUsedMiB << '/'
             << sample.gpu->memoryTotalMiB << " MiB";
    }
    else
    {
        text << ", GPU unavailable";
    }

    return text.str();
}
