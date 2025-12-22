#ifndef CONSUMER_H
#define CONSUMER_H

#include <QHash>
#include <QSharedPointer>
#include <QList>
#include "metric.h"
#include "usagegraph.h"

struct MetricStats
{
    double min = 0;
    double max = 0;
    double average = 0;
    double sum = 0;
};

struct ConsumerGraphData
{
    QSharedPointer<QCPDataContainer<QCPGraphData>> points;
    QSharedPointer<QCPDataContainer<QCPGraphData>> adaptedPoints;
    QSharedPointer<QCPDataContainer<QCPGraphData>> baseGraph;
    int pid;
    MetricStats resourceStats;
};

struct Consumer
{
    QHash<GraphDataMapKey, ConsumerGraphData> graphDataMap;
    QString consumerName;
    QString ecu;
    ModelItem *plottableModeltem = nullptr;
    QList<UsageGraph *> plottedGraphs = {};
};

#endif // CONSUMER_H
