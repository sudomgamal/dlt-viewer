#ifndef TESTINGFRAMEWORKPLUGINFORM_H
#define TESTINGFRAMEWORKPLUGINFORM_H

#include <QWidget>
#include "ui_testingframeworkpluginform.h"
#include "actionlistprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TestingFrameworkPluginForm; }
QT_END_NAMESPACE

class TestingFrameworkPluginForm : public QWidget
{
    Q_OBJECT

public:
    TestingFrameworkPluginForm(QWidget *parent = nullptr);
    ~TestingFrameworkPluginForm();

    void setConnections(QStringList list);
    void setDltControl(QDltControl control);
    ActionListProcessor processor;
private slots:
    void on_cbActionType_currentIndexChanged(int index);
    void on_btnAddAction_clicked();
    void on_btExecuteList_clicked();
    void on_btnClearActionQueue_clicked();
    void on_cbConnections_currentIndexChanged(int index);
    void on_lwActionList_doubleClicked(const QModelIndex &index);

    void actionStatusUpdated(int index, ActionStatus status);

private:
    Ui::TestingFrameworkPluginForm *ui;

    std::vector<TestAction> allTestActions;
    std::vector<TestAction> actionQueue;

    std::vector<TestAction> currentActionList; /*the list of actions to be executed*/
};
#endif // TESTINGFRAMEWORKPLUGINFORM_H
