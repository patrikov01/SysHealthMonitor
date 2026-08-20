#pragma once
#include "logger.h"

#include <atomic>
#include <chrono>
#include <string>

struct MonitorConfig
{
    std::chrono::seconds interval{2};
    unsigned int criticalSamples = 3;
    double maximumCpuTemperatureC = 85.0;
    double maximumCpuUsagePercent = 95.0;
    std::string logFile = "syshealth.log";
};

class HealthMonitor
{
public:
    HealthMonitor(MonitorConfig config);

    int run(const std::atomic<bool>& stopRequested);

private:
    bool isCritical(const HealthSample& sample) const;
    void sendNotification(const HealthSample& sample) const;
    static std::string describe(const HealthSample& sample);

    MonitorConfig config;
    Logger logger;
};