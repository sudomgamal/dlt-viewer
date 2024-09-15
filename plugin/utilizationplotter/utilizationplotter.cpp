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
    form = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
    prevProcStat = PidStatCpuDetails();
    lastTimeStamp = 0;
    dltFile = NULL;
}

UtilizationPlotter::~UtilizationPlotter()
{

}

QString UtilizationPlotter::name()
{
    return QString("Utilization Plotter Plugin");
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
    form = new UtilizationPlotterUI::Form();
    form->setMainTableView(m_mainTablView);
    return form;
}


void UtilizationPlotter::selectedIdxMsg(int , QDltMsg &) {
    //empty. Implemented because derived plugin interface functions are virtual.
}

void UtilizationPlotter::selectedIdxMsgDecoded(int , QDltMsg &){
    //empty. Implemented because derived plugin interface functions are virtual.
}

void UtilizationPlotter::initFileStart(QDltFile *file)
{
    dltFile = file;
    counterMessages = dltFile->size();
    prevProcStat = PidStatCpuDetails();
    lastTimeStamp = 0;
    form->reset();
}

void UtilizationPlotter::initMsg(int index, QDltMsg &msg)
{
    updateProcesses(index, msg);
}
void UtilizationPlotter::initMsgDecoded(int , QDltMsg &){
    //empty. Implemented because derived plugin interface functions are virtual.
}

void UtilizationPlotter::initFileFinish(){

}

void UtilizationPlotter::updateFileStart(){
}
void UtilizationPlotter::updateMsg(int index, QDltMsg &msg){

    if(!dltFile)
        return;

    updateProcesses(index, msg);
    counterMessages = dltFile->size();
}

void UtilizationPlotter::updateMsgDecoded(int , QDltMsg &){
    //empty. Implemented because derived plugin interface functions are virtual.
}
void UtilizationPlotter::updateFileFinish()
{


}


void UtilizationPlotter::updateProcesses(int , QDltMsg &msg)
{
    QDltArgument arg;
    int seq;
    if(!dltFile)
        return;
    QString strTs = QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
    double timestamp = strTs.toDouble();

    if(msg.getMode() == QDltMsg::DltModeVerbose)
    {
        counterVerboseMessages++;
    }
    if(msg.getMode() == QDltMsg::DltModeNonVerbose)
    {
        counterNonVerboseMessages++;
    }

    if(msg.getApid()=="SYS" && msg.getCtid()=="PROC" && msg.getEcuid()=="ART" && msg.getNumberOfArguments() > 2)
    {
        msg.getArgument(0,arg);
        msg.getArgument(2,arg);
        ProcPidStatDetails currentPidStat(timestamp, arg.toString());
        double userUtil = (currentPidStat.utime - m_pidStatMap[currentPidStat.pid].utime) /
                          (timestamp - m_pidStatMap[currentPidStat.pid].timestamp);
        double sysUtil = (currentPidStat.stime - m_pidStatMap[currentPidStat.pid].stime) /
                          (timestamp - m_pidStatMap[currentPidStat.pid].timestamp);
        double total = userUtil + sysUtil;
        if(total > m_pidStatMap[currentPidStat.pid].maxUtil)
        {
            currentPidStat.maxUtil = total;
        }
        else
        {
            currentPidStat.maxUtil = m_pidStatMap[currentPidStat.pid].maxUtil;
        }
        // form->setNice(currentPidStat.comm);
        form->addEntry(timestamp, total, currentPidStat.comm, currentPidStat.pid, currentPidStat.maxUtil);
        m_pidStatMap[currentPidStat.pid] = currentPidStat;
    }

    if(msg.getApid()=="SYS" && msg.getCtid()=="STAT" && msg.getEcuid()=="ART")
    {
        msg.getArgument(0,arg);
        seq = arg.toString().toInt();
        if( seq == PROC_STAT_NODE_1 || seq == PROC_STAT_NODE_UNDEFINE )
        {
            msg.getArgument(1,arg);
            PidStatCpuDetails currentProcStat(arg.toString());
            double value = ((currentProcStat.total - prevProcStat.total) -
                            (currentProcStat.totalIdle - prevProcStat.totalIdle))
                           / (currentProcStat.total - prevProcStat.total) * 100;
            form->addEntry(timestamp, value, "Overall", 0);
            value = ((currentProcStat.system - prevProcStat.system)
                    / (currentProcStat.total - prevProcStat.total)) * 100;
            form->addEntry(timestamp, value, "System Utilization", 0);
            value = ((currentProcStat.user - prevProcStat.user)
                     / (currentProcStat.total - prevProcStat.total)) * 100;
            form->addEntry(timestamp, value, "User Utilization", 0);
            prevProcStat = currentProcStat;
        }
    }


}
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(utilizationplotter, UtilizationPlotter);
#endif
