#ifndef TESTCASE_H
#define TESTCASE_H

#include <QObject>
#include <QDebug>

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
        actionIndex = 0;
        actionType = TestActionType::INVALID;
        injection.clear();
        message = MessageDetails{"", MessageStatus::PENDING, 0};
    }

    int actionIndex ;
    TestActionType actionType;
    QStringList injection;
    MessageDetails message;
    ActionStatus status;
};


class TestCase: public QObject
{
    Q_OBJECT
public:
    TestCase(QObject *parent = nullptr);
    TestCase(const TestCase& tc, QObject *parent = nullptr);
    TestCase(const QString& name, QObject *parent = nullptr);
    const std::vector<TestAction>& getTCActionList() const;
    bool addAction(TestAction testAction);
    bool removeAction(int actionIndex);

    QString name;
private:
    std::vector<TestAction> tcActionList;
};

#endif // TESTCASE_H
