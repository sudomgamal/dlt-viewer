/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file utilizationplotter.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>
#include "utilizationplotter.hpp"

#define PROC_STAT_NODE_1 1
#define PROC_STAT_NODE_UNDEFINE -1

UtilizationPlotter::UtilizationPlotter()
{
    m_form = NULL;
    m_dltFile = NULL;

    QThread *thrd = QThread::create([this]()
        {
            while(true)
            {
                m_msgQueueMtx.lock();
                if(!m_msgQueue.empty())
                {
                    auto item = m_msgQueue.dequeue();
                    updateProcesses(item.first, item.second);
                }
                m_msgQueueMtx.unlock();
                //FIXME: why sleep makes graph very slow?
                // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    );

    thrd->start();
}

UtilizationPlotter::~UtilizationPlotter()
{

}

QString UtilizationPlotter::name()
{
    return QString("Performance Graphs[BETA]");
}

QString UtilizationPlotter::pluginVersion(){
    return DLT_UTILIZATIONPLOTTER_PLUGIN_VERSION;
}

QString UtilizationPlotter::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString UtilizationPlotter::description()
{
    return QString();
}

QString UtilizationPlotter::error()
{
    return QString();
}

bool UtilizationPlotter::loadConfig(QString /* filename */)
{
    return true;
}

bool UtilizationPlotter::saveConfig(QString /* filename */)
{
    return true;
}

QStringList UtilizationPlotter::infoConfig()
{
    return QStringList();
}

QWidget* UtilizationPlotter::initViewer()
{
    m_form = new UtilizationPlotterUI::Form();
    return m_form;
}


void UtilizationPlotter::selectedIdxMsg(int , QDltMsg &) {
    //empty. Implemented because derived plugin interface functions are virtual.
}

void UtilizationPlotter::selectedIdxMsgDecoded(int , QDltMsg &){
    //empty. Implemented because derived plugin interface functions are virtual.
}

void UtilizationPlotter::initFileStart(QDltFile *file)
{
    m_dltFile = file;
    m_ecuResetDataMap.clear();
    m_msgQueueMtx.lock();
    m_msgQueue.clear();
    m_msgQueueMtx.unlock();
    m_form->reset();
}

void UtilizationPlotter::initMsg(int index, QDltMsg &msg)
{
    //called when loading offline files
    m_msgQueueMtx.lock();
    if(msg.getApid()=="SYS" && (msg.getCtid()=="PROC" || msg.getCtid()=="STAT" || msg.getCtid()=="MEM"))
    {
        m_msgQueue.enqueue(QPair<int, QDltMsg>(index, msg));
    }
    m_msgQueueMtx.unlock();
}
void UtilizationPlotter::initMsgDecoded(int , QDltMsg &){
    //empty. Implemented because derived plugin interface functions are virtual.
}

void UtilizationPlotter::initFileFinish(){

}

void UtilizationPlotter::updateFileStart(){
}
void UtilizationPlotter::updateMsg(int index, QDltMsg &msg)
{
    //called when connected to online ECU
    if(!m_dltFile)
        return;

    m_msgQueueMtx.lock();
    if(msg.getApid()=="SYS" && (msg.getCtid()=="PROC" || msg.getCtid()=="STAT" || msg.getCtid()=="MEM"))
    {
        m_msgQueue.enqueue(QPair<int, QDltMsg>(index, msg));
    }
    m_msgQueueMtx.unlock();
}

void UtilizationPlotter::updateMsgDecoded(int , QDltMsg &){
    //empty. Implemented because derived plugin interface functions are virtual.
}
void UtilizationPlotter::updateFileFinish()
{


}


void UtilizationPlotter::updateProcesses(int index, QDltMsg &msg)
{
    QDltArgument arg;
    int seq;
    QString ecu = msg.getEcuid();

    QString strTs = QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
    double currentMsgTimestamp = strTs.toDouble();
    bool resetDetected = false;

    if(msg.getApid()=="SYS" && msg.getCtid()=="MEM")
    {
        msg.getArgument(1,arg);
        static QRegularExpression regex(R"(^(\w+):\s+(\d+)\s+kB$)");
        if(m_ecuResetDataMap[ecu].powerCycles.empty() || m_ecuResetDataMap[ecu].powerCycles.empty() || (m_ecuResetDataMap[ecu].lastHandledTimestamp - currentMsgTimestamp) > 5)
        {
            qDebug() << "reset detected by MEM for ECU:" << ecu << "at" << currentMsgTimestamp << "lastHandledTimestamp:" << m_ecuResetDataMap[ecu].lastHandledTimestamp;
            m_ecuResetDataMap[ecu].powerCycles.push_back({});
            resetDetected = true;
        }

        double memAvailble = -1;
        for (auto const line : arg.toString().split('\n'))
        {
            QRegularExpressionMatch match = regex.match(line);

            if (match.hasMatch())
            {
                QString key = match.captured(1);
                int value = match.captured(2).toInt();
                m_ecuResetDataMap[ecu].powerCycles.back().memInfoMap[key] = value;
                // qDebug() << key << ":" << value << "kB";
            }
            else
            {
                // qDebug() << "No match for" << line;
            }
        }

        m_ecuResetDataMap[ecu].ramSize = m_ecuResetDataMap[ecu].powerCycles.back().memInfoMap["MemTotal"];
        memAvailble = m_ecuResetDataMap[ecu].powerCycles.back().memInfoMap["MemAvailable"];

        // qDebug() << "memTotal:memAvailble" << m_ecuResetDataMap[ecu].ramSize << ":" << memAvailble << "kB";

        double totalMemory = ((m_ecuResetDataMap[ecu].ramSize - memAvailble) / m_ecuResetDataMap[ecu].ramSize) * 100.0;
        if(totalMemory >= 0 && totalMemory <= 100)
            m_form->addEntry(currentMsgTimestamp, totalMemory,
                         index, TotalMemory, ecu, true, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, "Overall");

        resetDetected = false;

    }

    if(msg.getApid()=="SYS" && msg.getCtid()=="STAT")
    {
        msg.getArgument(0,arg);
        seq = arg.toString().toInt();
        if(seq == PROC_STAT_NODE_1 || seq == PROC_STAT_NODE_UNDEFINE)
        {
            if(m_ecuResetDataMap[ecu].powerCycles.empty() || (m_ecuResetDataMap[ecu].lastHandledTimestamp - currentMsgTimestamp) > 5)
            {
                qDebug() << "reset detected by STAT for ECU:" << ecu << "at" << currentMsgTimestamp << "lastHandledTimestamp:" << m_ecuResetDataMap[ecu].lastHandledTimestamp;
                m_ecuResetDataMap[ecu].powerCycles.push_back({});
                resetDetected = true;
            }

            msg.getArgument(1,arg);
            ProcStat currentProcStat(arg.toString());
            double overallTotalCpu = ((currentProcStat.total - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.total) -
                            (currentProcStat.totalIdle - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.totalIdle))
                           / (currentProcStat.total - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.total) * 100;
            double overallUserCpu = ((currentProcStat.user - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.user)
                                     / (currentProcStat.total - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.total)) * 100;
            double overallSystemCpu = ((currentProcStat.system - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.system)
                                       / (currentProcStat.total - m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat.total)) * 100;
            m_form->addEntry(currentMsgTimestamp, overallTotalCpu, index, TotalCpu, ecu, true, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, "Overall");
            resetDetected = false;
            m_form->addEntry(currentMsgTimestamp, overallSystemCpu, index, SystemCpu, ecu, true, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, "Overall");
            m_form->addEntry(currentMsgTimestamp, overallUserCpu, index, UserCpu, ecu, true, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, "Overall");
            //TOD: Add entries for memory usage
            m_ecuResetDataMap[ecu].powerCycles.back().latestProcStat = currentProcStat;
        }
    }

    if(msg.getApid()=="SYS" && msg.getCtid()=="PROC" && msg.getNumberOfArguments() > 2)
    {
        if(m_ecuResetDataMap[ecu].powerCycles.empty() || (m_ecuResetDataMap[ecu].lastHandledTimestamp - currentMsgTimestamp) > 5)
        {
            qDebug() << "reset detected by STAT for ECU:" << ecu << "at" << currentMsgTimestamp << "lastHandledTimestamp:" << m_ecuResetDataMap[ecu].lastHandledTimestamp;
            m_ecuResetDataMap[ecu].powerCycles.push_back({});
            resetDetected = true;
        }

        msg.getArgument(2,arg);
        ProcPidStat currentPidStat(currentMsgTimestamp, arg.toString());
        double userUtil = static_cast<double>((currentPidStat.utime - m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].utime)) /
                          (currentMsgTimestamp - m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].timestamp);
        double sysUtil = static_cast<double>((currentPidStat.stime - m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].stime)) /
                          (currentMsgTimestamp - m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].timestamp);
        double total = userUtil + sysUtil;
        if(total > 100 || sysUtil > 100 || userUtil > 100)
        {
            qDebug() << (ecu + "_" + currentPidStat.comm) <<  "unsusual values:" << total << sysUtil << userUtil << currentMsgTimestamp
                     << m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].timestamp << currentPidStat.utime
                     << m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].utime
                     << currentPidStat.stime << m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid].stime;
        }
        // Add entries for memory usage
        // double totalMemoryUsage = currentPidStat.totalMemoryUsage; // Replace with actual calculation
        double rssMemoryUsage =static_cast<double>(currentPidStat.rss) / 1024 / 1024; //MB
        double virtualMemoryUsage = static_cast<double>(currentPidStat.vsize) / 1024 / 1024; //MB

        if(total >= 0 && total <= 100)
            m_form->addEntry(currentMsgTimestamp, total, index, TotalCpu, ecu, false, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, currentPidStat.comm, currentPidStat.pid);
        resetDetected = false;
        if(sysUtil >= 0 && sysUtil <= 100)
            m_form->addEntry(currentMsgTimestamp, sysUtil, index, SystemCpu, ecu, false, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, currentPidStat.comm, currentPidStat.pid);
        if(userUtil >= 0 && userUtil <= 100)
            m_form->addEntry(currentMsgTimestamp, userUtil, index, UserCpu, ecu, false, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, currentPidStat.comm, currentPidStat.pid);
        if(rssMemoryUsage >= 0 && rssMemoryUsage <= 100)
            m_form->addEntry(currentMsgTimestamp, rssMemoryUsage, index, RssMemory, ecu, false, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, currentPidStat.comm, currentPidStat.pid);
        m_form->addEntry(currentMsgTimestamp, virtualMemoryUsage, index, VirtualMemory, ecu, false, resetDetected, m_ecuResetDataMap[ecu].powerCycles.size()-1, currentPidStat.comm, currentPidStat.pid);
        m_ecuResetDataMap[ecu].powerCycles.back().pidStatMap[currentPidStat.pid] = currentPidStat;
    }

    m_ecuResetDataMap[ecu].lastHandledTimestamp = currentMsgTimestamp;
}
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(utilizationplotter, UtilizationPlotter);
#endif
