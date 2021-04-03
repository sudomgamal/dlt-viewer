#include "testingframeworkplugin.h"

TestingFrameworkPlugin::TestingFrameworkPlugin(QObject *parent) : QObject(parent)
{

}

QString TestingFrameworkPlugin::name()
{
    return pluginName;
}

QString TestingFrameworkPlugin::description()
{
    return pluginDescription;
}

QString TestingFrameworkPlugin::pluginVersion()
{
    return TESTING_FRAMEWORK_PLUGIN_VERSION;
}

QString TestingFrameworkPlugin::pluginInterfaceVersion()
{
    return TESTING_FRAMEWORK_PLUGIN_INTERFACE_VERSION;
}

QString TestingFrameworkPlugin::error()
{
    return "";
}

bool TestingFrameworkPlugin::loadConfig(QString filename)
{
    Q_UNUSED(filename);
    return true;
}

bool TestingFrameworkPlugin::saveConfig(QString filename)
{
    Q_UNUSED(filename);
    return true;
}

QStringList TestingFrameworkPlugin::infoConfig()
{
    return QStringList();
}

QWidget* TestingFrameworkPlugin::initViewer()
{
    pluginForm = new TestingFrameworkPluginForm();
    connect(this, SIGNAL(newMsg(QDltMsg)), &pluginForm->processor, SLOT(messageReceived(QDltMsg)));
    return pluginForm;
}

void TestingFrameworkPlugin::initFileStart(QDltFile *file)
{
    Q_UNUSED(file);
}

void TestingFrameworkPlugin::initFileFinish()
{

}

void TestingFrameworkPlugin::initMsg(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    Q_UNUSED(msg);
}

void TestingFrameworkPlugin::initMsgDecoded(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    Q_UNUSED(msg);
}

void TestingFrameworkPlugin::updateFileStart()
{

}

void TestingFrameworkPlugin::updateMsg(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    Q_UNUSED(msg);
}

void TestingFrameworkPlugin::updateMsgDecoded(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    emit newMsg(msg);
}

void TestingFrameworkPlugin::updateFileFinish()
{

}

void TestingFrameworkPlugin::selectedIdxMsg(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    Q_UNUSED(msg);
}

void TestingFrameworkPlugin::selectedIdxMsgDecoded(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    Q_UNUSED(msg);
}
bool TestingFrameworkPlugin::initControl(QDltControl *control)
{
    pluginForm->processor.setDltControl(control);
    qDebug() << control;
    return true;
}

bool TestingFrameworkPlugin::initConnections(QStringList list)
{
    pluginForm->setConnections(list);

    return false;
}

bool TestingFrameworkPlugin::controlMsg(int , QDltMsg &)
{
    return false;
}

bool TestingFrameworkPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname)
{
    Q_UNUSED(index);
    Q_UNUSED(connectionState);
    Q_UNUSED(hostname);
    return true;
}

bool TestingFrameworkPlugin::autoscrollStateChanged(bool enabled)
{
    Q_UNUSED(enabled);
    return true;
}

void TestingFrameworkPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

void TestingFrameworkPlugin::initMainTableView(QTableView* pTableView)
{
    Q_UNUSED(pTableView);
}

void TestingFrameworkPlugin::configurationChanged()
{}
