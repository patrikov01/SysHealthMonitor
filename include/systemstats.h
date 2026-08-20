#pragma once
#include <cstdint>
#include <optional>

struct CpuTimes
{
    std::uint64_t idle = 0;
    std::uint64_t total = 0;
};

struct GpuStats
{
    double temperatureC = 0.0;
    double utilizationPercent = 0.0;
    double memoryUsedMiB = 0.0;
    double memoryTotalMiB = 0.0;
};

struct HealthSample
{
    double cpuUsagePercent = 0.0;
    std::optional<double> cpuTemperatureC;
    std::optional<GpuStats> gpu;
};

class SystemStats
{
public:
    static std::optional<CpuTimes> readCpuTimes();
    static double calculateCpuUsage(const CpuTimes& previous, const CpuTimes& current);
    static std::optional<double> readCpuTemperature();
    static std::optional<GpuStats> readGpuStats();
};
