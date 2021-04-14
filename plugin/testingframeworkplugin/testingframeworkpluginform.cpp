#include "testingframeworkpluginform.h"
#include <QDebug>
#include <QProcess>

TestingFrameworkPluginForm::TestingFrameworkPluginForm(QWidget *parent)
    : QWidget(parent)
    , tcListFile(tcListFilePath)
    , ui(new Ui::TestingFrameworkPluginForm)
{
    ui->setupUi(this);
    ui->grbExpectationDetails->setEnabled(false);
    currentTestCase = nullptr;
    if (!tcListFile.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug()<< "Cannot open file: " << tcListFile << "for reading";
    QXmlStreamReader stream(&tcListFile);
    int index = 0;
    if (stream.readNextStartElement())
    {
        if (stream.name() == "testcaselist")
        {
            while (stream.readNextStartElement())
            {
//                qDebug()<< "READER: NAME -> " << stream.name();
                if (stream.name() == "testcase")
                {
                    qDebug()<< "READER: new testcase";
                    TestCase newTestCase;
                    testCaseList.push_back(newTestCase);
                    qDebug()<< "testCaseList.size(): " << testCaseList.size();
                    while (stream.readNextStartElement())
                    {
//                        qDebug()<< "READER: NAME ->>> " << stream.name();
                        if (stream.name() == "tcname")
                        {
                            testCaseList.back().name = stream.readElementText();
                            index = 0;
                            qDebug()<< "READER: new testcase -> " << testCaseList.back().name;
                        }
                        else if (stream.name() == "inj")
                        {
                            QStringList inj = stream.readElementText().split(',');
                            TestAction act;
                            act.actionType = TestActionType::SEND_INJECTION;
                            act.injection = inj;
                            act.actionIndex = index;
                            testCaseList.back().addAction(act);
                            index++;
                            qDebug()<< "READER: injection -> " << inj;
                        }
                        else if (stream.name() == "msg")
                        {
                            QString strTimeOut = stream.attributes().value("timeout").toString();
                            TestAction act;
                            act.actionType = TestActionType::WAIT_FOR_MESSAGE;
                            act.message.text =  stream.readElementText();
                            act.message.timeout = strTimeOut.toInt();
                            act.actionIndex = index;
                            testCaseList.back().addAction(act);
                            index++;
                            qDebug()<< "READER: message -> " << act.message.text << " <- for " << act.message.timeout << "ms" ;
                        }
                    }
                }
            }
        }
    }
    tcListFile.close();

    qDebug()<<"All data read :VVVVVVVVVVVVVVVVVVVVV";
    for (auto &tc : testCaseList)
    {
        ui->cbTCList->addItem(tc.name);
        qDebug()<<"Actions of " << tc.name;
        for (auto &act : tc.getTCActionList())
        {
            if (act.actionType == TestActionType::SEND_INJECTION)
            {
                qDebug() << "Action: send ["<< act.injection[InjectionIdx::TITLE] <<"]";
            }
            else if (act.actionType == TestActionType::WAIT_FOR_MESSAGE)
            {
                qDebug() << "Action: wait for msg [" << act.message.text <<"] for "
                         << act.message.timeout/1000 << " seconds";
            }
        }
    }

    connect(&processor, SIGNAL(informActionStatus(int, ActionStatus)), this, SLOT(actionStatusUpdated(int, ActionStatus)));
    connect(&processor, SIGNAL(processingFinished(bool)), ui->btExecuteList, SLOT(setEnabled(bool)));
}

TestingFrameworkPluginForm::~TestingFrameworkPluginForm()
{
    if (!tcListFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QXmlStreamWriter stream(&tcListFile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("testcaselist");

    for (auto testCase = testCaseList.cbegin(); testCase < testCaseList.cend(); ++testCase)
    {
        qDebug() << "adding test case" << testCase->name;
        stream.writeStartElement("testcase");
        stream.writeTextElement("tcname", testCase->name);
        for (auto action = testCase->getTCActionList().crbegin(); action<testCase->getTCActionList().crend(); ++action)
        {
            if (action->actionType == TestActionType::SEND_INJECTION)
            {
                qDebug() << "adding inj" << action->injection.join(",");
                stream.writeTextElement("inj", action->injection.join(","));
            }
            else if (action->actionType == TestActionType::WAIT_FOR_MESSAGE)
            {
                qDebug() << "adding msg" << action->message.text;
                stream.writeStartElement("msg");
                stream.writeAttribute("timeout", QString::number(action->message.timeout));
                stream.writeCharacters(action->message.text);
                stream.writeEndElement();
            }
        }

        stream.writeEndElement(); //testCase->name
    }

    stream.writeEndElement(); //testcaselist
    stream.writeEndDocument();
    tcListFile.close();
    qDebug() << "dying";
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
    if (currentTestCase == nullptr)
    {
        qDebug()<< "currentTestCase = nullptr";
        return;
    }

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
        tAction.actionIndex = currentTestCase->getTCActionList().size();
        currentTestCase->addAction(tAction);
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
        tAction.actionIndex = currentTestCase->getTCActionList().size();
        currentTestCase->addAction(tAction);
    }
    else
    {
        qDebug() << "Unknown index ["<< ui->cbActionType->currentIndex() << "].  Please report a bug.";
    }


    qDebug()<<"All Actions:__________________________";
    for (auto &act : currentTestCase->getTCActionList())
    {
        if (act.actionType == TestActionType::SEND_INJECTION)
        {
            qDebug() << "Action: send ["<< act.injection[InjectionIdx::TITLE] <<"]";
        }
        else if (act.actionType == TestActionType::WAIT_FOR_MESSAGE)
        {
            qDebug() << "Action: wait for msg [" << act.message.text <<"] for " << act.message.timeout/1000 << " seconds";
        }
    }
}

void TestingFrameworkPluginForm::on_btExecuteList_clicked()
{
    for (int i=0; i < ui->lwActionQueue->count(); ++i)
    {
        ui->lwActionQueue->item(i)->setBackgroundColor(Qt::white);
    }
    //    processor.setActionList(actionQueue);
    processor.processActions();
}

void TestingFrameworkPluginForm::on_btnClearActionQueue_clicked()
{
    //    actionQueue.clear();
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

void TestingFrameworkPluginForm::actionStatusUpdated(int index, ActionStatus status)
{
    qDebug() << "Action[" << index << "] is updated with status: " << status;
    switch (status)
    {
    case ActionStatus::ACTION_WAITING:
        ui->lwActionList->item(index)->setBackgroundColor(Qt::yellow);
        break;
    case ActionStatus::ACTION_SUCCESSFUL:
        ui->lwActionList->item(index)->setBackgroundColor(Qt::green);
        break;
    case ActionStatus::ACTION_FAILED:
        ui->lwActionList->item(index)->setBackgroundColor(Qt::red);
        break;
    case ActionStatus::ACTION_PENDING:
    default:
        break;
    }
}

void TestingFrameworkPluginForm::on_btnAdbForwardPort_clicked()
{
#ifdef Q_OS_WIN
    QString program("cmd.exe");
    QStringList parameters;
    parameters << " /k adb wait-for-device & adb forward tcp:3490 tcp:3490";
    QProcess::startDetached(program, parameters);
#elif Q_OS_LINUX
    QString program("/bin/sh");
    QStringList parameters;
    parameters << " -c adb wait-for-device && adb forward tcp:3490 tcp:3490";
    QProcess::startDetached(program, parameters);
#endif
}

void TestingFrameworkPluginForm::on_btnTCExcecute_clicked()
{

    for (int i=0; i < ui->lwActionList->count(); ++i)
    {
        ui->lwActionList->item(i)->setBackgroundColor(Qt::white);
    }
    processor.setActionList(currentTestCase->getTCActionList());
    processor.processActions();
}

void TestingFrameworkPluginForm::on_cbTCList_currentIndexChanged(int index)
{
    int tcListSize = testCaseList.size();
    qDebug() << "index: " << index << " testCaseList.size(): " << QString::number(tcListSize);

    if (tcListSize <= index)
    {
        qDebug() << "Adding: " << ui->cbTCList->currentText() << " test case";
        TestCase newTestCase(ui->cbTCList->currentText());
        testCaseList.push_back(newTestCase);
        currentTestCase = &(*(testCaseList.end()-1));
        qDebug() << currentTestCase->name << " added";
    }
    else
    {
        currentTestCase = &testCaseList[index];
        qDebug() << "Test case: " << currentTestCase->name << " already exists";
    }

    ui->lwActionList->clear();
    for (auto  action = currentTestCase->getTCActionList().crbegin();
         action < currentTestCase->getTCActionList().crend(); ++action)
    {
        if (action->actionType == TestActionType::SEND_INJECTION)
        {
            QStringList itemDetails;
            itemDetails << "Send injection: <" << action->injection[InjectionIdx::TITLE] << ">";
            ui->lwActionList->addItem(itemDetails.join(""));
            ui->lwActionList->item(ui->lwActionList->count()-1)->setToolTip(action->injection[InjectionIdx::APPID]+ " " +
                    action->injection[InjectionIdx::CTXID]+" " +
                    action->injection[InjectionIdx::SRVID]+" " +
                    action->injection[InjectionIdx::DATA]);
        }
        else if(action->actionType == TestActionType::WAIT_FOR_MESSAGE)
        {
            QStringList itemDetails;
            itemDetails << "Wait for message: " << action->message.text;

            if(action->message.timeout > 0)
            {
                itemDetails << " within " << QString::number(action->message.timeout/1000) << " seconds";
            }

            ui->lwActionList->addItem(itemDetails.join(""));
            ui->lwActionList->item(ui->lwActionList->count()-1)->setToolTip(action->message.text);
        }
        else
        {
            qDebug() << "Unknown type ["<< action->actionType << "].  Please report a bug.";
        }
    }
}

void TestingFrameworkPluginForm::on_cbTCList_currentIndexChanged(const QString &arg1)
{
    qDebug() << arg1;
}

void TestingFrameworkPluginForm::on_btnTCAddToQueue_clicked()
{

}

void TestingFrameworkPluginForm::keyPressEvent ( QKeyEvent * event )
{
    if(event->key() == Qt::Key_Delete)
    {
        if(ui->lwActionList->hasFocus())
        {
            auto list = ui->lwActionList->selectionModel()->selectedRows();
            for (auto item : qAsConst(list))
            {
                qDebug() << "Removing " << item.row();
                currentTestCase->removeAction(item.row());
                ui->lwActionList->model()->removeRow(item.row());
            }
        }
    }
}
