#ifndef MODELLITEM_H
#define MODELLITEM_H

#include <QStandardItem>
#include <QHash>
#include <QSharedPointer>
#include "metric.h"

class Consumer;

class ModelItem : public QStandardItem
{
public:
    enum SortMeasure
    {
        Average = 0,
        Max,
    };

    QString consumersMapKey;
    bool isOverallConsumer = false;
    static Metric systemResource;
    static SortMeasure sortType;
    static QHash<QString, QSharedPointer<Consumer>> *consumersMapPtr;
    static QHash<QPair<QString, int>, bool> *ecuResetsEnableMap;

protected:
    bool operator<(const QStandardItem &other) const override;
};

#endif // MODELLITEM_H
