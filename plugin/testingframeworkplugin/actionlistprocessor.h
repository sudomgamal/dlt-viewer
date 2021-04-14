#ifndef ACTIONLISTPROCESSOR_H
#define ACTIONLISTPROCESSOR_H

#include <QStringList>
#include <vector>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include "testcase.h"
#include "plugininterface.h"
#include <algorithm>

class ActionListProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ActionListProcessor(QObject *parent = nullptr);
    bool addAction(TestAction testAction);
    void clearActionList();
    bool setActionList(const std::vector<TestAction> &actions);
    bool processActions();
    void setDltControl(QDltControl *ctrl);
    void setConnectionIndex(int idx);

private:
    std::vector<TestAction> vActions;
    TestAction *currentAction;
    QTimer *processTimer;
    const int msgWaitingInterval = 100;
    int timerPeriod;
    int msgTickCounter;
    QDltControl *dltControl;
    int connectionIdx;
    bool sendInjection(QStringList injection);

private slots:
    void processTick();

signals:
    void informActionStatus(int, ActionStatus);
    void processingFinished(bool);

public slots:
    void messageReceived(QDltMsg msg);
};

#endif // ACTIONLISTPROCESSOR_H
