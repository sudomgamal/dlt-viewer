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

struct ProcStat
{
    ProcStat() = default;
    ProcStat(const QString &statLine)
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

struct ProcPidStat
{
    ProcPidStat() = default;
    ProcPidStat(double ts, const QString &statLine): timestamp(ts)
    {
        auto strList = statLine.split(' ');
        if(strList.size() < 52)
        {
            return;
        }
        pid = strList[0].toInt();
        comm = strList[1];
        comm.remove(')');
        comm.remove('(');
        state = strList[2];
        ppid = strList[3].toInt();
        pgrp = strList[4].toInt();
        session = strList[5].toInt();
        tty_nr = strList[6].toInt();
        tpgid = strList[7].toInt();
        flags = strList[8].toUInt();
        minflt = strList[9].toULongLong();
        cminflt = strList[10].toULongLong();
        majflt = strList[11].toULongLong();
        cmajflt = strList[12].toULongLong();
        utime = strList[13].toULongLong();
        stime = strList[14].toULongLong();
        cutime = strList[15].toLongLong();
        cstime = strList[16].toLongLong();
        priority = strList[17].toInt();
        nice = strList[18].toInt();
        num_threads = strList[19].toInt();
        itrealvalue = strList[20].toLongLong();
        starttime = strList[21].toULongLong();
        vsize = strList[22].toULongLong();
        rss = strList[23].toLongLong();
        rsslim = strList[24].toULongLong();
        startcode = strList[25].toULongLong();
        endcode = strList[26].toULongLong();
        startstack = strList[27].toULongLong();
        kstkesp = strList[28].toULongLong();
        kstkeip = strList[29].toULongLong();
        signal = strList[30].toULongLong();
        blocked = strList[31].toULongLong();
        sigignore = strList[32].toULongLong();
        sigcatch = strList[33].toULongLong();
        wchan = strList[34].toULongLong();
        nswap = strList[35].toULongLong();
        cnswap = strList[36].toULongLong();
        exit_signal = strList[37].toInt();
        processor = strList[38].toInt();
        rt_priority = strList[39].toInt();
        policy = strList[40].toInt();
        delayacct_blkio_ticks = strList[41].toULongLong();
        guest_time = strList[42].toULongLong();
        cguest_time = strList[43].toLongLong();
        start_data = strList[44].toULongLong();
        end_data = strList[45].toULongLong();
        start_brk = strList[46].toULongLong();
        arg_start = strList[47].toULongLong();
        arg_end = strList[48].toULongLong();
        env_start = strList[49].toULongLong();
        env_end = strList[50].toULongLong();
        exit_code = strList[51].toInt();
    }
    double timestamp;

    int pid;
    QString comm;
    QString state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned int flags;
    unsigned long long minflt;
    unsigned long long cminflt;
    unsigned long long majflt;
    unsigned long long cmajflt;
    unsigned long long utime;
    unsigned long long stime;
    long long cutime;
    long long cstime;
    int priority;
    int nice;
    int num_threads;
    long long itrealvalue;
    unsigned long long starttime;
    unsigned long long vsize;
    long long rss;
    unsigned long long rsslim;
    unsigned long long startcode;
    unsigned long long endcode;
    unsigned long long startstack;
    unsigned long long kstkesp;
    unsigned long long kstkeip;
    unsigned long long signal;
    unsigned long long blocked;
    unsigned long long sigignore;
    unsigned long long sigcatch;
    unsigned long long wchan;
    unsigned long long nswap;
    unsigned long long cnswap;
    int exit_signal;
    int processor;
    int rt_priority;
    int policy;
    unsigned long long delayacct_blkio_ticks;
    unsigned long long guest_time;
    long long cguest_time;
    unsigned long long start_data;
    unsigned long long end_data;
    unsigned long long start_brk;
    unsigned long long arg_start;
    unsigned long long arg_end;
    unsigned long long env_start;
    unsigned long long env_end;
    int exit_code;
};

struct PowerCycleData
{
    ProcStat latestProcStat {};
    QMap<int, ProcPidStat> pidStatMap {};
    QMap<QString, int> memInfoMap;
};

struct EcuData
{
    double resetIdx = 0;
    double ramSize = 0;
    double lastHandledTimestamp = 0;
    QVector<PowerCycleData> powerCycles;
};

class UtilizationPlotter : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
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

    bool initControl(QDltControl *ctrl) final override { if(m_form) m_form->setDltControl(ctrl) ; return true; }
    bool initConnections(QStringList) final override { return true; }
    bool controlMsg(int, QDltMsg &) final override { return true; }
    bool stateChanged(int , QDltConnection::QDltConnectionState ,QString ) final override{return true;}
    bool autoscrollStateChanged(bool) final override { return true; }

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    void initMainTableView(QTableView* pMainTableView) override {m_mainTablView = pMainTableView;
        // if(form) form->setMainTableView(m_mainTablView);
    }
    void initMessageDecoder(QDltMessageDecoder* ) override {}
    void configurationChanged() override {}
#endif

    /* internal variables */
    UtilizationPlotterUI::Form *m_form;
    void show(bool value);
    void updateProcesses(int index, QDltMsg &msg);
    QDltControl *m_dltConrol;
private:
    QMap<QString, EcuData> m_ecuResetDataMap;

    QDltFile *m_dltFile;
    QTableView* m_mainTablView;
    QString m_errorText;
    QQueue<QPair<int, QDltMsg>> m_msgQueue;
    QMutex m_msgQueueMtx;
};

#endif // UtilizationPlotter_HPP
