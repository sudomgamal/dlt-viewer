#include "testingframeworkpluginform.h"
#include <QDebug>

TestingFrameworkPluginForm::TestingFrameworkPluginForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestingFrameworkPluginForm)
{
    ui->setupUi(this);
    ui->grbExpectationDetails->setEnabled(false);

    connect(&processor, SIGNAL(informActionStatus(int, ActionStatus)), this, SLOT(actionStatusUpdated(int, ActionStatus)));
}

TestingFrameworkPluginForm::~TestingFrameworkPluginForm()
{
    qDebug()<< "dying";
    delete ui;
}


void TestingFrameworkPluginForm::on_cbActionType_currentIndexChanged(int index)
{
    if(index == TestActionType::SEND_INJECTION)
    {
        ui->grbInjectionDetails->setEnabled(true);
        ui->grbExpectationDetails->setEnabled(false);
    }
    else if(index == TestActionType::WAIT_FOR_MESSAGE)
    {
        ui->grbInjectionDetails->setEnabled(false);
        ui->grbExpectationDetails->setEnabled(true);
    }
    else
    {
        qDebug() << "Unknown index ["<< index << "].  Please report a bug.";
    }
}

void TestingFrameworkPluginForm::on_btnAddAction_clicked()
{
    QStringList itemDetails;
    TestAction tAction;

    if (ui->cbActionType->currentIndex() == TestActionType::SEND_INJECTION)
    {
        tAction.actionType = TestActionType::SEND_INJECTION;
        tAction.injection << ui->leInjTitle->text() << ui->leInjAppId->text()
                          << ui->leInjCtxId->text()<< ui->spInjSrvId->text() << ui->leInjData->text();

        itemDetails << "Send injection: <" << ui->leInjTitle->text() << ">";
        ui->lwActionList->addItem(itemDetails.join(""));
        ui->lwActionList->item(ui->lwActionList->count()-1)->setToolTip(tAction.injection[InjectionIdx::APPID]+ " " +
                tAction.injection[InjectionIdx::CTXID]+" " +
                tAction.injection[InjectionIdx::SRVID]+" " +
                tAction.injection[InjectionIdx::DATA]);
        allTestActions.insert(allTestActions.begin(), tAction);
    }
    else if(ui->cbActionType->currentIndex() == TestActionType::WAIT_FOR_MESSAGE)
    {
        itemDetails << "Wait for message: " << ui->leMsgText->text();

        tAction.actionType = TestActionType::WAIT_FOR_MESSAGE;
        tAction.message.text = ui->leMsgText->text();
        tAction.message.timeout = ui->spMsgTimeout->value() * 1000;

        if(ui->spMsgTimeout->value() > 0)
        {
            itemDetails << " within " << ui->spMsgTimeout->text() << " seconds";
        }

        ui->lwActionList->addItem(itemDetails.join(""));
        ui->lwActionList->item(ui->lwActionList->count()-1)->setToolTip(tAction.message.text);
        allTestActions.insert(allTestActions.begin(), tAction);
    }
    else
    {
        qDebug() << "Unknown index ["<< ui->cbActionType->currentIndex() << "].  Please report a bug.";
    }


    qDebug()<<"All Actions:__________________________";
    for (auto &act : allTestActions)
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
}

void TestingFrameworkPluginForm::on_btExecuteList_clicked()
{
    processor.setActionList(actionQueue);
    processor.processActions();
}

void TestingFrameworkPluginForm::on_btnClearActionQueue_clicked()
{
    actionQueue.clear();
    ui->lwActionQueue->clear();
}

void TestingFrameworkPluginForm::setConnections(QStringList list)
{
    ui->cbConnections->clear();
    ui->cbConnections->insertItems(0, list);
}

void TestingFrameworkPluginForm::on_cbConnections_currentIndexChanged(int index)
{
    processor.setConnectionIndex(index);
}

void TestingFrameworkPluginForm::on_lwActionList_doubleClicked(const QModelIndex &index)
{
    int idx = index.row();
    /*allTestActions is reversed*/
    actionQueue.insert(actionQueue.begin(), allTestActions.at(allTestActions.size() - idx - 1));
    ui->lwActionQueue->addItem(ui->lwActionList->item(idx)->text());

    qDebug()<<"Queue Actions:__________________________";
    for (auto &act : actionQueue)
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
    qDebug()<<"Queue Actions:***********************";
}
void TestingFrameworkPluginForm::actionStatusUpdated(int index, ActionStatus status)
{
    qDebug() << "Action[" << index << "] is updated with status: " << status;
    switch (status)
    {
    case ActionStatus::ACTION_WAITING:
        ui->lwActionQueue->item(index)->setBackgroundColor(Qt::yellow);
        break;
    case ActionStatus::ACTION_SUCCESSFUL:
        ui->lwActionQueue->item(index)->setBackgroundColor(Qt::green);
        break;
    case ActionStatus::ACTION_FAILED:
        ui->lwActionQueue->item(index)->setBackgroundColor(Qt::red);
        break;
    case ActionStatus::ACTION_PENDING:
    default:
        break;
    }
}
