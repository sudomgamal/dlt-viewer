#include "modelitem.h"
#include "consumer.h"

Metric ModelItem::systemResource = Metric::TotalCpu;
ModelItem::SortMeasure ModelItem::sortType = ModelItem::SortMeasure::Average;
QHash<QString, QSharedPointer<Consumer>> *ModelItem::consumersMapPtr = nullptr;
QHash<QPair<QString, int>, bool> *ModelItem::ecuResetsEnableMap = nullptr;

bool ModelItem::operator<(const QStandardItem &other) const
{
    if (this->model()->sortRole() == Qt::DisplayRole)
    {
        return (text().compare(other.text(), Qt::CaseInsensitive) < 0);
    }

    ModelItem *rhs = (ModelItem *)&other;
    ModelItem *lhs = (ModelItem *)this;

    if (!consumersMapPtr || consumersMapPtr->value(lhs->consumersMapKey, nullptr) == nullptr ||
        consumersMapPtr->value(rhs->consumersMapKey, nullptr) == nullptr ||
        consumersMapPtr->value(lhs->consumersMapKey)->ecu != consumersMapPtr->value(rhs->consumersMapKey)->ecu)
    {
        return false;
    }

    bool ret = false;
    QHashIterator<QPair<QString, int>, bool> ecuPsItem(*ecuResetsEnableMap);
    ecuPsItem.toFront();
    while (ecuPsItem.hasNext())
    {
        ecuPsItem.next();
        if (consumersMapPtr->value(lhs->consumersMapKey)->ecu != ecuPsItem.key().first)
        {
            continue;
        }

        if (sortType == Average)
        {
            ret = consumersMapPtr->value(lhs->consumersMapKey)
                      ->graphDataMap.value({ecuPsItem.key().second, systemResource})
                      .resourceStats.average <
                  consumersMapPtr->value(rhs->consumersMapKey)
                      ->graphDataMap.value({ecuPsItem.key().second, systemResource})
                      .resourceStats.average;
        }
        else
        {
            ret = consumersMapPtr->value(lhs->consumersMapKey)
                      ->graphDataMap.value({ecuPsItem.key().second, systemResource})
                      .resourceStats.max <
                  consumersMapPtr->value(rhs->consumersMapKey)
                      ->graphDataMap.value({ecuPsItem.key().second, systemResource})
                      .resourceStats.max;
        }
    }

    return ret;
}
