#include "testcase.h"

TestCase::TestCase(QObject *parent): QObject(parent)
{
    tcActionList.clear();
    name.clear();
}

TestCase::TestCase(const TestCase &tc, QObject *parent): QObject(parent)
{
    name = tc.name;
    tcActionList = tc.tcActionList;
}

TestCase::TestCase(const QString& tcname, QObject *parent): QObject(parent)
{
    tcActionList.clear();
    name = tcname;
}

const std::vector<TestAction>& TestCase::getTCActionList() const
{
    return tcActionList;
}

bool TestCase::addAction(TestAction testAction)
{
    int taCount = tcActionList.size();
    if (testAction.actionIndex < taCount)
    {
        qDebug()<<"Action already added..";
        return false;
    }
    else
    {
        qDebug()<<"Adding action to test case ["<<name<<"]";
        tcActionList.insert(tcActionList.begin(), testAction);
        return true;
    }
}

bool TestCase::removeAction(int actionIndex)
{
    int taCount = tcActionList.size();
    if (actionIndex < taCount)
    {
        tcActionList.erase(tcActionList.cend() - actionIndex - 1);
        return true;
    }
    else
    {
        return false;
        qDebug() << __FUNCTION__ << "Error: Item index out of range";
    }
}
