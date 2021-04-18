#ifndef TESTINGFRAMEWORKPLUGINFORM_H
#define TESTINGFRAMEWORKPLUGINFORM_H

#include <QWidget>
#include "ui_testingframeworkpluginform.h"
#include "actionlistprocessor.h"
#include <QKeyEvent>
#include "msgfilterdialog.h"

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
    const QString tcListFilePath = "TestCases.xml";
    QFile tcListFile;
private slots:
    void on_cbActionType_currentIndexChanged(int index);
    void on_btnAddAction_clicked();
    void on_btExecuteList_clicked();
    void on_btnClearActionQueue_clicked();
    void on_cbConnections_currentIndexChanged(int index);
    void actionStatusUpdated(int index, ActionStatus status);
    void on_btnAdbForwardPort_clicked();

    void on_btnTCExcecute_clicked();

    void on_cbTCList_currentIndexChanged(int index);

    void on_cbTCList_currentIndexChanged(const QString &arg1);

    void on_btnTCAddToQueue_clicked();
    void on_btnSetFilter_clicked();
    void filterDialogRead(FilterDialog &dlg, QDltFilter *filter);
    void handleProcessingOngoing(bool isInProgress);

protected:
    void keyPressEvent ( QKeyEvent * event );

private:
    bool loadTestCasesFromXML();
    Ui::TestingFrameworkPluginForm *ui;

    std::vector<TestCase> testCaseList;
    std::vector<TestCase*> testCaseQueue; /*queued to be executed togeter*/
    TestCase* currentTestCase;
    QDltFilter *currentMessageFilter;
};
#endif // TESTINGFRAMEWORKPLUGINFORM_H
