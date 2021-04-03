#ifndef ACTIONLISTPROCESSOR_H
#define ACTIONLISTPROCESSOR_H

#include <QStringList>
#include <vector>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include "plugininterface.h"
#include <algorithm>

enum MessageStatus
{
    PENDING,
    WAITING,
    NOT_RECEIVED,
    RECEIVED
};

struct MessageDetails
{
    QString text;
    MessageStatus status;
    int timeout;
};

enum ActionStatus
{
    ACTION_PENDING,
    ACTION_WAITING,
    ACTION_FAILED,
    ACTION_SUCCESSFUL
};

/*Index names for QStringList injection*/
enum InjectionIdx
{
    TITLE, /*injection title*/
    APPID, /*application ID*/
    CTXID, /*context ID*/
    SRVID, /*service ID*/
    DATA   /*data*/
};

enum TestActionType
{
    SEND_INJECTION,
    WAIT_FOR_MESSAGE,
    INVALID
};

struct TestAction
{
    explicit TestAction()
    {
        actionType = TestActionType::INVALID;
        injection = QStringList();
        message = MessageDetails{"", MessageStatus::PENDING, 0};
    }

    TestActionType actionType;
    QStringList injection;
    MessageDetails message;
    ActionStatus status;
};

class ActionListProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ActionListProcessor(QObject *parent = nullptr);
    bool addAction(TestAction testAction);
    void clearActionList();
    bool setActionList(std::vector<TestAction> &actions);
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

public slots:
    void messageReceived(QDltMsg msg);
};

#endif // ACTIONLISTPROCESSOR_H
