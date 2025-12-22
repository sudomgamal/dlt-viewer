#include "usagegraph.h"

// Constructor
UsageGraph::UsageGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, Metric type)
    : QCPGraph(keyAxis, valueAxis), metric(type)
{
}

// Method to get the map key
GraphDataMapKey UsageGraph::mapKey()
{
    return {powerCycle, metric};
}
