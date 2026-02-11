#ifndef SRC_DRAWINFO_H
#define SRC_DRAWINFO_H

#include <string>

struct DrawInfo
{
    std::wstring timeString;
    std::wstring cpuUsage;
    std::wstring memoryUsage;
    std::wstring networkUsage;
};


#endif //SRC_DRAWINFO_H