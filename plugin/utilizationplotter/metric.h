#ifndef METRIC_H
#define METRIC_H

#include <cstdint>
#include <QHash>

enum Metric : uint8_t
{
    TotalCpu = 0,
    SystemCpu,
    UserCpu,
    RssMemory,
    TotalMemory,
    VirtualMemory,
    UNKOWN
};

struct PointMetadata
{
    Metric metric;
    int powerCycle = 0;
    int pid;
    int dltIdx = 0;
};

struct GraphDataMapKey
{
    int powerCycle;
    Metric metric;

    bool operator==(const GraphDataMapKey &other) const
    {
        return powerCycle == other.powerCycle && metric == other.metric;
    }
};

// Hash function for GraphDataMapKey
uint qHash(const GraphDataMapKey &key, uint seed);

#endif // METRIC_H
