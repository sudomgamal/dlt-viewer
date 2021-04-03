#ifndef TESTINGFRAMEWORKPLUGIN_H
#define TESTINGFRAMEWORKPLUGIN_H

#include <QObject>
#include "testingframeworkpluginform.h"
#include "plugininterface.h"

/*Plugin details*/
#define TESTING_FRAMEWORK_PLUGIN_VERSION "1.0.0"
#define TESTING_FRAMEWORK_PLUGIN_INTERFACE_VERSION PLUGIN_INTERFACE_VERSION

class TestingFrameworkPlugin : public QObject , QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
public:
    explicit TestingFrameworkPlugin(QObject *parent = nullptr);
#ifdef QT5
    Q_PLUGIN_METADATA(IID "metadata.TestingFrameworkPlugin")
#endif

    /*QDLTPluginInterface function implementation. The description of each function is commented in plugininterface.h*/
    QString name();
    QString description();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginViewerInterface */
    QWidget* initViewer();
    void initFileStart(QDltFile *file);
    void initFileFinish();
    void initMsg(int index, QDltMsg &msg);
    void initMsgDecoded(int index, QDltMsg &msg);
    void updateFileStart();
    void updateMsg(int index, QDltMsg &msg);
    void updateMsgDecoded(int index, QDltMsg &msg);
    void updateFileFinish();
    void selectedIdxMsg(int index, QDltMsg &msg);
    void selectedIdxMsgDecoded(int index, QDltMsg &msg);

    /* QDltPluginControlInterface */
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);
    bool controlMsg(int index, QDltMsg &msg);
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname);
    bool autoscrollStateChanged(bool enabled);
    void initMessageDecoder(QDltMessageDecoder* pMessageDecoder);
    void initMainTableView(QTableView* pTableView);
    void configurationChanged();


    TestingFrameworkPluginForm *pluginForm;

signals:
    void newMsg(QDltMsg msg);

private:
    QString pluginName = "Testing Framework";
    QString pluginDescription = "Used to automate testing";
};

#endif // TESTINGFRAMEWORKPLUGIN_H
