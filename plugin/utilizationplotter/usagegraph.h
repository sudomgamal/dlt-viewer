#ifndef USAGEGRAPH_H
#define USAGEGRAPH_H

#include "qcustomplot.h"
#include "metric.h"

class ModelItem;

class UsageGraph : public QCPGraph
{
    Q_OBJECT

public:
    explicit UsageGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, Metric type);
    GraphDataMapKey mapKey();

    ModelItem *plottableModeltem;
    int powerCycle = 0;
    int pid;
    Metric metric;
};

#endif // USAGEGRAPH_H
