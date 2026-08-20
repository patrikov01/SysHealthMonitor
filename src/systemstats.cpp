#include "systemstats.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::optional<CpuTimes> SystemStats::readCpuTimes()
{
    std::ifstream input("/proc/stat");
    std::string line;

    if (!std::getline(input, line))
    {
        return std::nullopt;
    }

    std::istringstream stream(line);
    std::string label;
    std::vector<std::uint64_t> values;
    std::uint64_t value = 0;

    stream >> label;
    while (stream >> value)
    {
        values.push_back(value);
    }

    if (label != "cpu" || values.size() < 4)
    {
        return std::nullopt;
    }

    CpuTimes times;
    // Fields 3 and 4 are idle and iowait; both count as time when the CPU was not busy.
    times.idle = values[3];
    if (values.size() > 4)
    {
        times.idle += values[4];
    }

    for (const std::uint64_t item : values)
    {
        times.total += item;
    }

    return times;
}

double SystemStats::calculateCpuUsage(const CpuTimes& previous, const CpuTimes& current)
{
    // /proc/stat contains counters since boot, so only the change between samples is useful
    const std::uint64_t totalDelta = current.total - previous.total;
    const std::uint64_t idleDelta = current.idle - previous.idle;

    if (totalDelta == 0 || idleDelta > totalDelta)
    {
        return 0.0;
    }

    return 100.0 * static_cast<double>(totalDelta - idleDelta) / static_cast<double>(totalDelta);
}

std::optional<double> SystemStats::readCpuTemperature()
{
    namespace fs = std::filesystem;

    // There can be several thermal zones. The highest readable value is the safest summary
    std::optional<double> highestTemperature;
    const fs::path thermalRoot("/sys/class/thermal");
    std::error_code error;

    if (!fs::exists(thermalRoot, error))
    {
        return std::nullopt;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(thermalRoot, error))
    {
        if (error || !entry.is_directory())
        {
            continue;
        }

        const std::string name = entry.path().filename().string();
        if (name.rfind("thermal_zone", 0) != 0)
        {
            continue;
        }

        std::ifstream input(entry.path() / "temp");
        double millidegrees = 0.0;
        if (input >> millidegrees)
        {
            const double temperature = millidegrees / 1000.0;
            if (!highestTemperature || temperature > *highestTemperature)
            {
                highestTemperature = temperature;
            }
        }
    }

    return highestTemperature;
}

std::optional<GpuStats> SystemStats::readGpuStats()
{
    const char* command =
        "nvidia-smi --query-gpu=temperature.gpu,utilization.gpu,memory.used,memory.total "
        "--format=csv,noheader,nounits 2>/dev/null";
    FILE* pipe = popen(command, "r");

    if (pipe == nullptr)
    {
        return std::nullopt;
    }

    std::array<char, 256> buffer{};
    const bool readLine = std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr;
    const int status = pclose(pipe);

    if (!readLine || status != 0)
    {
        return std::nullopt;
    }

    // Replacing commas lets a regular string stream parse the four CSV columns.
    std::string line(buffer.data());
    std::replace(line.begin(), line.end(), ',', ' ');

    GpuStats stats;
    std::istringstream stream(line);
    if (!(stream >> stats.temperatureC >> stats.utilizationPercent
                 >> stats.memoryUsedMiB >> stats.memoryTotalMiB))
    {
        return std::nullopt;
    }

    return stats;
}
