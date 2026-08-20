#pragma once
#include "systemstats.h"
#include <fstream>
#include <string>

class Logger
{
public:
    Logger(const std::string& filePath);

    bool isOpen() const;
    void write(const HealthSample& sample);

private:
    std::ofstream output;
};