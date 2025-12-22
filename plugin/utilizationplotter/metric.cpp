#include "metric.h"
// Hash function for GraphDataMapKey
uint qHash(const GraphDataMapKey &key, uint seed)
{
    return ::qHash(key.powerCycle, seed) ^ ::qHash(static_cast<int>(key.metric), seed);
}
