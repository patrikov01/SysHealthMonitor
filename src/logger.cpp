#include "logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& filePath)
    : output(filePath, std::ios::app)
{
}

bool Logger::isOpen() const
{
    return output.is_open();
}

void Logger::write(const HealthSample& sample)
{
    const std::time_t now = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now());

    output << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
            << ',' << std::fixed << std::setprecision(1)
            << sample.cpuUsagePercent << ',';

    if (sample.cpuTemperatureC)
    {
        output << *sample.cpuTemperatureC;
    }

    // Missing sensor values remain empty so every log line keeps the same CSV layout.
    output << ',';
    if (sample.gpu)
    {
        output << sample.gpu->temperatureC << ','
               << sample.gpu->utilizationPercent << ','
               << sample.gpu->memoryUsedMiB << '/'
               << sample.gpu->memoryTotalMiB << " MiB";
    }
    else
    {
        output << ",,";
    }

    output << '\n';
    output.flush();
}
