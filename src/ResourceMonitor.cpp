#include "ResourceMonitor.h"

#include <stdexcept>
#include <iostream>
#include <format>
#include <vector>

namespace
{
std::wstring formatProcessorTime(double value)
{
    return std::format(L"CPU: {:>5.1f}%", value);
}

std::wstring formatMemoryBytes(long long bytes)
{
    constexpr long long KB = 1024;
    constexpr long long MB = KB * 1024;
    constexpr long long GB = MB * 1024;

    if (bytes >= GB)
    {
        return std::format(L"mem: {:>5.1f}GB", static_cast<double>(bytes) / GB);
    }
    else if (bytes >= MB)
    {
        return std::format(L"mem: {:>5.1f}MB", static_cast<double>(bytes) / MB);
    }
    else
    {
        return std::format(L"mem: {:>5.1f}KB", static_cast<double>(bytes) / KB);
    }
}

std::wstring formatNetworkBytesPerSec(long long bytesPerSec)
{
    constexpr long long Kb = 128;
    constexpr long long Mb = Kb * 1024;
    constexpr long long Gb = Mb * 1024;

    if (bytesPerSec >= Gb)
    {
        return std::format(L"net: {:>5.1f}Gbps", static_cast<double>(bytesPerSec) / Gb);
    }
    else if (bytesPerSec >= Mb)
    {
        return std::format(L"net: {:>5.1f}Mbps", static_cast<double>(bytesPerSec) / Mb);
    }
    else
    {
        return std::format(L"net: {:>5.1f}Kbps", static_cast<double>(bytesPerSec) / Kb);
    }
}

}

ResourceMonitor::ResourceMonitor()
{
    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &m_query);
    if (status != ERROR_SUCCESS || m_query == nullptr)
    {
        throw std::runtime_error("failed to open PDH query");
    }

    status = PdhAddCounter(m_query, CPU_COUNTER_PATH, 0, &m_cpuCounter);
    if (status != ERROR_SUCCESS || m_cpuCounter == nullptr)
    {
        throw std::runtime_error("failed to add CPU counter");
    }

    status = PdhAddCounter(m_query, MEMORY_COUNTER_PATH, 0, &m_memoryCounter);
    if (status != ERROR_SUCCESS || m_memoryCounter == nullptr)
    {
        throw std::runtime_error("failed to add Memory counter");
    }

    status = PdhAddCounter(m_query, NETWORK_COUNTER_PATH, 0, &m_networkCounter);
    if (status != ERROR_SUCCESS || m_networkCounter == nullptr)
    {
        throw std::runtime_error("failed to add Network counter");
    }
}

ResourceMonitor::~ResourceMonitor()
{
    if (m_query)
    {
        PdhCloseQuery(m_query);
        m_query = nullptr;
    }
}

DrawInfo ResourceMonitor::collect() const
{
    if (!m_query || !m_cpuCounter || !m_memoryCounter || !m_networkCounter)
    {
        return DrawInfo{
            .timeString = L"",
            .cpuUsage = formatProcessorTime(0),
            .memoryUsage = formatMemoryBytes(0),
            .networkUsage = formatNetworkBytesPerSec(0),
        };
    }

    PDH_STATUS status = PdhCollectQueryData(m_query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "failed to collect query data: " << status << std::endl;
        return DrawInfo{
            .timeString = L"",
            .cpuUsage = formatProcessorTime(0),
            .memoryUsage = formatMemoryBytes(0),
            .networkUsage = formatNetworkBytesPerSec(0),
        };
    }

    DWORD bufSize = 0;
    DWORD itemCount = 0;

    // CPU
    PdhGetFormattedCounterArray(
        m_cpuCounter,
        PDH_FMT_DOUBLE,
        &bufSize,
        &itemCount,
        nullptr
    );
    if (bufSize == 0 || itemCount == 0)
    {
        return DrawInfo{
            .timeString = L"",
            .cpuUsage = formatProcessorTime(0),
            .memoryUsage = formatMemoryBytes(0),
            .networkUsage = formatNetworkBytesPerSec(0),
        };
    }

    std::vector<std::byte> buf(bufSize);
    PPDH_FMT_COUNTERVALUE_ITEM items = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(buf.data());
    PdhGetFormattedCounterArray(
        m_cpuCounter,
        PDH_FMT_DOUBLE,
        &bufSize,
        &itemCount,
        items
    );
    double cpuUsage = items[0].FmtValue.doubleValue;

    // Memory
    bufSize = 0;
    itemCount = 0;
    PdhGetFormattedCounterArray(
        m_memoryCounter,
        PDH_FMT_LARGE,
        &bufSize,
        &itemCount,
        nullptr
    );
    if (bufSize == 0 || itemCount == 0)
    {
        return DrawInfo{
            .timeString = L"",
            .cpuUsage = formatProcessorTime(cpuUsage),
            .memoryUsage = formatMemoryBytes(0),
            .networkUsage = formatNetworkBytesPerSec(0),
        };
    }

    std::vector<std::byte> bufMem(bufSize);
    PPDH_FMT_COUNTERVALUE_ITEM itemsMem = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(bufMem.data());
    PdhGetFormattedCounterArray(
        m_memoryCounter,
        PDH_FMT_LARGE,
        &bufSize,
        &itemCount,
        itemsMem
    );
    long long memoryBytes = itemsMem[0].FmtValue.largeValue;

    // Network
    bufSize = 0;
    itemCount = 0;
    PdhGetFormattedCounterArray(
        m_networkCounter,
        PDH_FMT_LARGE,
        &bufSize,
        &itemCount,
        nullptr
    );
    if (bufSize == 0 || itemCount == 0)
    {
        return DrawInfo{
            .timeString = L"",
            .cpuUsage = formatProcessorTime(cpuUsage),
            .memoryUsage = formatMemoryBytes(memoryBytes),
            .networkUsage = formatNetworkBytesPerSec(0),
        };
    }

    std::vector<std::byte> bufNet(bufSize);
    PPDH_FMT_COUNTERVALUE_ITEM itemsNet = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(bufNet.data());
    PdhGetFormattedCounterArray(
        m_networkCounter,
        PDH_FMT_LARGE,
        &bufSize,
        &itemCount,
        itemsNet
    );
    long long networkBytesPerSec = 0;
    for (DWORD i = 0; i < itemCount; i++)
    {
        networkBytesPerSec += itemsNet[i].FmtValue.largeValue;
    }


    return DrawInfo{
        .timeString = L"",
        .cpuUsage = formatProcessorTime(cpuUsage),
        .memoryUsage = formatMemoryBytes(memoryBytes),
        .networkUsage = formatNetworkBytesPerSec(networkBytesPerSec),
    };
}

