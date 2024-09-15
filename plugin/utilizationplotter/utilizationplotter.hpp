/**
 * @licence app begin@
 * Copyright (C) Mohammed Gamal
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file utilizationplotter.hpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef UTILIZATIONPLOTTER_HPP
#define UTILIZATIONPLOTTER_HPP

#include <QObject>
#include "plugininterface.h"
#include "form.h"

#define DLT_UTILIZATIONPLOTTER_PLUGIN_VERSION "1.0.0"

struct PidStatCpuDetails
{
    PidStatCpuDetails() = default;
    PidStatCpuDetails(const QString &statLine)
    {
        QTextStream stream(statLine.toUtf8());
        QString devNull;
        stream >> devNull;
        stream >> user;
        stream >> nice;
        stream >> system;
        stream >> idle;
        stream >> iowait;
        stream >> irq;
        stream >> softirq;
        stream >> steal;
        stream >> guest;
        stream >> guest_nice;
        totalIdle = idle + iowait;
        total = user + nice + system + irq + softirq + steal + totalIdle;
    }

    int user = 0;
    int nice = 0;
    int system = 0;
    int idle = 0;
    int iowait = 0;
    int irq = 0;
    int softirq = 0;
    int steal = 0;
    int guest = 0;
    int guest_nice = 0;
    double total = 0;
    double totalIdle = 0;
};

struct ProcPidStatDetails
{
    ProcPidStatDetails() = default;
    ProcPidStatDetails(double ts, const QString &statLine): timestamp(ts)
    {
        auto strList = statLine.split(' ');
        if(strList.size() < 40)
        {
            return;
        }
        pid = strList[0].toInt();
        comm = strList[1];
        comm.remove(')');
        comm.remove('(');
        utime = strList[13].toInt();
        stime = strList[14].toInt();
        cutime = strList[15].toInt();
        cstime = strList[16].toInt();
        rss = strList[23].toInt();
        nswap = strList[35].toInt();
        cnswap = strList[36].toInt();
    }
    double timestamp;
    int pid = 0;
    QString comm = 0;
    int utime = 0;
    int stime = 0;
    int cutime = 0;
    int cstime = 0;
    int rss = 0;
    int nswap = 0;
    int cnswap = 0;
    double maxUtil = 0;
};

class UtilizationPlotter : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    // Q_INTERFACES(QDltPluginControlInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.UtilizationPlotter")
#endif

public:
    UtilizationPlotter();
    ~UtilizationPlotter();

    /* QDLTPluginInterface interface */
    QString name() final override;
    QString pluginVersion() final override;
    QString pluginInterfaceVersion() final override;
    QString description() final override;
    QString error() final override;
    bool loadConfig(QString filename) final override;
    bool saveConfig(QString filename) final override;
    QStringList infoConfig() final override;

    /* QDltPluginViewerInterface */
    QWidget* initViewer() final override;
    void initFileStart(QDltFile *file) final override;
    void initFileFinish() final override;
    void initMsg(int index, QDltMsg &msg) final override;
    void initMsgDecoded(int index, QDltMsg &msg) final override;
    void updateFileStart() final override;
    void updateMsg(int index, QDltMsg &msg) final override;
    void updateMsgDecoded(int index, QDltMsg &msg) final override;
    void updateFileFinish() final override;
    void selectedIdxMsg(int index, QDltMsg &msg) final override;
    void selectedIdxMsgDecoded(int index, QDltMsg &msg) final override;

    bool initControl(QDltControl *) final override { return true; }
    bool initConnections(QStringList) final override { return true; }
    bool controlMsg(int, QDltMsg &) final override { return true; }
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname) final override{}
    bool autoscrollStateChanged(bool) final override { return true; }

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    void initMainTableView(QTableView* pMainTableView) override {m_mainTablView = pMainTableView;
        if(form) form->setMainTableView(m_mainTablView);
    }
    void initMessageDecoder(QDltMessageDecoder* pMessageDecoder) override {}
    void configurationChanged() override {}
#endif

    /* internal variables */
    UtilizationPlotterUI::Form *form;
    int counterMessages;
    int counterNonVerboseMessages;
    int counterVerboseMessages;

    PidStatCpuDetails prevProcStat;
    QMap<int, ProcPidStatDetails> m_pidStatMap;
    unsigned int lastTimeStamp;

    void show(bool value);

    void updateProcesses(int index, QDltMsg &msg);

private:
    QDltFile *dltFile;
    QTableView* m_mainTablView;
    QString errorText;
};

#endif // UtilizationPlotter_HPP
