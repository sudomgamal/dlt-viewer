#include "actionlistprocessor.h"

ActionListProcessor::ActionListProcessor(QObject *parent) : QObject(parent)
{
    processTimer = new QTimer(this);
    vActions = std::vector<TestAction>();
    timerPeriod = 0;
    msgTickCounter = 0;
    currentAction = nullptr;
    dltControl = nullptr;
    connect(processTimer, SIGNAL(timeout()), this, SLOT(processTick()));
}

bool ActionListProcessor::addAction(TestAction testAction)
{
    vActions.push_back(testAction);
    return true;
}

void ActionListProcessor::clearActionList()
{
    vActions.clear();
}

bool ActionListProcessor::setActionList(const std::vector<TestAction> &actions)
{
    vActions.clear();
    if(actions.empty())
    {
        qErrnoWarning("actions are empty ... not executing");
        return false;
    }
    vActions = actions;
    qDebug()<<"setActionList:*********************";
    for (auto &act : vActions)
    {
        if (act.actionType == TestActionType::SEND_INJECTION)
        {
            qDebug() << "Action: send ["<< act.injection[InjectionIdx::TITLE] <<"]";
        }
        else if (act.actionType == TestActionType::WAIT_FOR_MESSAGE)
        {
            qDebug() << "Action: wait for msg [" << act.message.text <<"] for " << act.message.timeout << " seconds";
        }
    }
    return true;
}

void ActionListProcessor::setDltControl(QDltControl *ctrl)
{
    dltControl = ctrl;
}

void ActionListProcessor::setConnectionIndex(int idx)
{
    connectionIdx = idx;
}

void ActionListProcessor::processTick()
{
    static int actionIndex = 0;
    if(vActions.empty() || (actionIndex >= (int)vActions.size()))
    {
        qDebug() << "ActionListProcessor: Nothing to process.";
        processTimer->stop();
        timerPeriod = 0;
        actionIndex = 0;
        emit processingFinished(true);
        return;
    }

    emit processingFinished(false);
    currentAction = &vActions.at(vActions.size() - actionIndex - 1);

    if (currentAction && currentAction->actionType == TestActionType::SEND_INJECTION)
    {
        if (sendInjection(currentAction->injection))
        {
            currentAction->status = ActionStatus::ACTION_SUCCESSFUL;
            qDebug()<< "HOORAAY: "  << "Sending injection <" << currentAction->injection[InjectionIdx::TITLE] << ">";
        }
        else
        {
            currentAction->status = ActionStatus::ACTION_FAILED;
            qDebug()<< "BOOOOOO: "  << "Sending injection <" << currentAction->injection[InjectionIdx::TITLE] << ">";
        }

        timerPeriod = 0;
        emit informActionStatus(actionIndex, currentAction->status);
        actionIndex++;
    }
    else if(currentAction && currentAction->actionType == TestActionType::WAIT_FOR_MESSAGE)
    {
        if (currentAction->message.status == MessageStatus::PENDING)
        {
            qDebug() << "Waiting for " << currentAction->message.text << " to come within " <<
                        currentAction->message.timeout << " ms";

            msgTickCounter = 0;
            timerPeriod = msgWaitingInterval;
            currentAction->message.status = MessageStatus::WAITING;
            currentAction->status = ActionStatus::ACTION_WAITING;
            emit informActionStatus(actionIndex, currentAction->status);
        }
        else if(currentAction->message.status == MessageStatus::WAITING)
        {
            msgTickCounter += msgWaitingInterval;
            if (currentAction->message.timeout < msgTickCounter)
            {
                timerPeriod = 0;
                currentAction->message.status = MessageStatus::NOT_RECEIVED;
                currentAction->status = ActionStatus::ACTION_FAILED;

                emit informActionStatus(actionIndex, currentAction->status);
                actionIndex++;
            }
        }
        else if(currentAction->message.status == MessageStatus::RECEIVED)
        {
            /*the MessageStatus::RECEIVED state is set by the
             * ActionListProcessor::messageReceived callback*/
            timerPeriod = 0;
            currentAction->status = ActionStatus::ACTION_SUCCESSFUL;

            emit informActionStatus(actionIndex, currentAction->status);
            actionIndex++;
        }
    }

    processTimer->start(timerPeriod);
}

bool ActionListProcessor::processActions()
{
    emit processingFinished(false);
    processTimer->start(timerPeriod);
    return true;
}

void ActionListProcessor::messageReceived(QDltMsg msg)
{
    if(currentAction && currentAction->actionType == TestActionType::WAIT_FOR_MESSAGE)
    {
        QDltArgument arg;
        msg.getArgument(0,arg);
        QString msgText = arg.toString();
        if (currentAction->message.status == MessageStatus::WAITING)
        {

            if (msg.getPayload().size() && ! currentAction->message.text.compare(msgText, Qt::CaseSensitive))
            {
                qDebug()<< "HOORAAY: " << msgText;
                currentAction->message.status = MessageStatus::RECEIVED;
            }
            else
            {
                qDebug()<<"BOOOOO: " << msgText;
            }
        }
    }
}

bool ActionListProcessor::sendInjection(QStringList injection)
{
    if(injection.empty() || !dltControl)
    {
        return false;
    }

    QString appId = injection[InjectionIdx::APPID];
    QString ctxId = injection[InjectionIdx::CTXID];
    QString srvId = injection[InjectionIdx::SRVID];
    QString payload = injection[InjectionIdx::DATA];

    if (payload.isEmpty())
    {
        payload.append('\0');
    }
    dltControl->sendInjection(connectionIdx, appId, ctxId, srvId.toInt(), payload.toLatin1());

    return true;
}
