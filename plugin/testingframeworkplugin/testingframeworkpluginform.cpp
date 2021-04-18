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
    loadTestCasesFromXML();

    connect(&processor, SIGNAL(informActionStatus(int, ActionStatus)), this, SLOT(actionStatusUpdated(int, ActionStatus)));
    connect(&processor, SIGNAL(processingOngoing(bool)), this, SLOT(handleProcessingOngoing(bool)));
}

bool TestingFrameworkPluginForm::loadTestCasesFromXML()
{
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
                        //qDebug()<< "READER: NAME ->>> " << stream.name();
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
                            stream.readNextStartElement();/*<text>*/
                            act.message.text =  stream.readElementText();
                            qDebug()<< "READER: NAME -> " << stream.name();
                            act.message.timeout = strTimeOut.toInt();
                            act.actionIndex = index;
                            QDltFilter filter;
                            bool filterRead = false;

                            while (!filterRead)
                            {
                                stream.readNext();
                                //qDebug()<<"READER: current msg name >>>>> "<< stream.name();
                                if(stream.isStartElement())
                                {
                                    if(stream.name() == QString("filter"))
                                    {
                                        filter.clear();
                                    }
                                    filter.LoadFilterItem(stream);
                                }
                                if(stream.isEndElement() && stream.name() == QString("filter"))
                                {
                                    qDebug()<<"READER: setting filter "<< stream.name();
                                    filterRead = true;
                                    stream.skipCurrentElement();
                                }
                            }
                            QDltFilter *filter_new = new QDltFilter();
                            *filter_new = filter;
                            act.message.msgFilter = filter_new;

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

    /*ptint the list and update the UI*/
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
    return true;
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
        for (auto action = testCase->getTCActionList().rbegin(); action<testCase->getTCActionList().rend(); ++action)
        {
            if (action->actionType == TestActionType::SEND_INJECTION)
            {
                qDebug() << "adding inj" << action->injection.join(",");
                stream.writeTextElement("inj", action->injection.join(","));
            }
            else if (action->actionType == TestActionType::WAIT_FOR_MESSAGE)
            {
                if (action->message.msgFilter)
                {
                    qDebug() << "adding msg" << action->message.text;
                    stream.writeStartElement("msg");
                    stream.writeAttribute("timeout", QString::number(action->message.timeout));
                    stream.writeTextElement("text", action->message.text);
                    stream.writeStartElement("filter");
                    action->message.msgFilter->SaveFilterItem(stream);
                    stream.writeEndElement();//filter
                    stream.writeEndElement();//msg
                }
                else
                {
                    qDebug()<< "Not writing message duee to empty filter";
                }
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

        /*currentMessageFilter should be updated by the filter dialog in on_btnSetFilter_clicked
            if not, create a tmp one with the message text as payload*/
        if (currentMessageFilter != nullptr)
        {
            tAction.message.msgFilter = currentMessageFilter;
            currentMessageFilter = nullptr;
        }
        else
        {
            QDltFilter *tmpFilter = new QDltFilter();
            tmpFilter->type = (QDltFilter::FilterType)(QDltFilter::FilterType::positive);
            tmpFilter->enableFilter = true;
            tmpFilter->enablePayload = true;
            tmpFilter->enableMarker = false;
            tAction.message.msgFilter = tmpFilter;
        }
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
}

void TestingFrameworkPluginForm::on_btnClearActionQueue_clicked()
{
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
        ui->lblStatus->setText(QString("Waiting for message %1").arg(currentTestCase->getAction(index).message.text));
        ui->lwActionList->item(index)->setBackgroundColor(Qt::yellow);
        break;
    case ActionStatus::ACTION_SUCCESSFUL:
        ui->lwActionList->item(index)->setBackgroundColor(Qt::green);
        ui->lblStatus->setText(QString("Action completed successfully"));
        break;
    case ActionStatus::ACTION_FAILED:
        ui->lblStatus->setText(QString("Action Failed"));
        ui->lwActionList->item(index)->setBackgroundColor(Qt::red);
        break;
    case ActionStatus::ACTION_PENDING:
        ui->lwActionList->item(index)->setBackgroundColor(Qt::yellow);
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
    testCaseQueue.insert(testCaseQueue.begin(), &testCaseList[ui->cbTCList->currentIndex()]);
    ui->lwActionQueue->addItem(ui->cbTCList->currentText());
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

void TestingFrameworkPluginForm::on_btnSetFilter_clicked()
{
    /* show filter dialog */
    FilterDialog dlg;
    //    dlg.setEnableEcuId(!msg.getEcuid().isEmpty());
    //    dlg.setEcuId(msg.getEcuid());
    //    dlg.setEnableApplicationId(!msg.getApid().isEmpty());
    //    dlg.setApplicationId(msg.getApid());
    //    dlg.setEnableContextId(!msg.getCtid().isEmpty());
    //    dlg.setContextId(msg.getCtid());
    //    dlg.setHeaderText(msg.toStringHeader());
    dlg.setPayloadText(ui->leMsgText->text());
    dlg.setEnablePayloadText(true);
    //    dlg.setMessageId_min(msg.getMessageId());
    //    dlg.setMessageId_max(0);

    if(dlg.exec()==1)
    {
        QDltFilter *newFilter = new QDltFilter();
        currentMessageFilter = newFilter;
        filterDialogRead(dlg, currentMessageFilter);
    }
}

void TestingFrameworkPluginForm::filterDialogRead(FilterDialog &dlg,QDltFilter *filter)
{
    filter->type = (QDltFilter::FilterType)(dlg.getType());
    filter->name = ui->leMsgText->text();
    filter->ecuid = dlg.getEcuId();
    filter->apid = dlg.getApplicationId();
    filter->ctid = dlg.getContextId();
    filter->header = dlg.getHeaderText();
    filter->payload = dlg.getPayloadText();
    filter->regex_search = dlg.getRegexSearchText();
    filter->regex_replace = dlg.getRegexReplaceText();
    filter->enableRegexp_Appid = dlg.getEnableRegexp_Appid();
    filter->enableRegexp_Context = dlg.getEnableRegexp_Context();
    filter->enableRegexp_Header = dlg.getEnableRegexp_Header();
    filter->enableRegexp_Payload = dlg.getEnableRegexp_Payload();
    filter->ignoreCase_Header = dlg.getIgnoreCase_Header();
    filter->ignoreCase_Payload = dlg.getIgnoreCase_Payload();
    filter->enableFilter = true;
    filter->enableEcuid = dlg.getEnableEcuId();
    filter->enableApid = dlg.getEnableApplicationId();
    filter->enableCtid = dlg.getEnableContextId();
    filter->enableHeader = dlg.getEnableHeaderText();
    filter->enablePayload = dlg.getEnablePayloadText();
    filter->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
    filter->enableLogLevelMax = dlg.getEnableLogLevelMax();
    filter->enableLogLevelMin = dlg.getEnableLogLevelMin();
    filter->enableMarker = false;
    filter->enableMessageId = dlg.getEnableMessageId();
    filter->enableRegexSearchReplace = dlg.getEnableRegexSearchReplace();
    filter->logLevelMax = dlg.getLogLevelMax();
    filter->logLevelMin = dlg.getLogLevelMin();
    filter->messageIdMax=dlg.getMessageId_max();
    filter->messageIdMin=dlg.getMessageId_min();
}
void TestingFrameworkPluginForm::handleProcessingOngoing(bool isInProgress)
{
    ui->tabWidget->setEnabled(!isInProgress);
}
