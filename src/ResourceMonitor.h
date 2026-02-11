#ifndef SRC_RESOURCEMONITOR_H
#define SRC_RESOURCEMONITOR_H

#include <windows.h>
#include <pdh.h>

#include "DrawInfo.h"

class ResourceMonitor
{
public:
    ResourceMonitor();
    ~ResourceMonitor();

    DrawInfo collect() const;

private:
    HQUERY m_query = nullptr;
    HCOUNTER m_cpuCounter = nullptr;
    HCOUNTER m_memoryCounter = nullptr;
    HCOUNTER m_networkCounter = nullptr;

    const wchar_t *CPU_COUNTER_PATH = L"\\Processor(_Total)\\% Processor Time";
    const wchar_t *MEMORY_COUNTER_PATH = L"\\Memory\\Committed Bytes";
    const wchar_t *NETWORK_COUNTER_PATH = L"\\Network Interface(*)\\Bytes Received/sec";
};


#endif //SRC_RESOURCEMONITOR_H