/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file injectionsplugin.cpp
 * For further information see https://github.com/sudomgamal/dlt-viewer
 * @licence end@
 */

#include <QtGui>
#include <QtXml/QtXml>
#include <QMessageBox>

#include "injectionsplugin.h"

InjectionsPlugin::InjectionsPlugin()
{
    form = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
    dltFile = 0;
    dltControl = 0;
    ecuList = 0;
}

InjectionsPlugin::~InjectionsPlugin()
{
    emit unloadRequested();
}

QString InjectionsPlugin::name()
{
    return QString("Injections");
}

QString InjectionsPlugin::pluginVersion()
{
    return INJECTIONS_PLUGIN_VERSION;
}

QString InjectionsPlugin::pluginInterfaceVersion()
{
    return PLUGIN_INTERFACE_VERSION;
}

QString InjectionsPlugin::description()
{
    return QString("Load, Save and easily inject messages");
}

QString InjectionsPlugin::error()
{
    return QString();
}

bool InjectionsPlugin::loadConfig(QString dirPath)
{
    //(void)filename; // unused because we have to implement the pure virtual loadConfig(QString)
    m_injectionGroups.clear();

    //const QString dirPath = filename;
    //m_advancedOptions.updateConfigurationPath(dirPath);
    QDir directory(dirPath);   // get object to the the subfolder "injections" in the dlt folder

    if(!directory.exists())
    {
        qDebug() << "Folder doesn't exist, creating directory: " << dirPath;
        directory.mkdir(dirPath);

        const QString injectionsFile = dirPath + "/injections.csv";
        QFile injectionFile(injectionsFile);
        qDebug() << "creating injections file: " << injectionsFile;
        if(injectionFile.open(QIODevice::ReadWrite))
        {
            qDebug() << "File created.";
        }
        else
        {
            qDebug() << "Failed to created " << injectionsFile;
        }
    }

    QStringList nameFilters;
    nameFilters.append("*csv"); // creat a list of all the files that we want to load, which are any csv (*csv)
    QStringList csvFiles = directory.entryList(nameFilters, QDir::Files);  // load all csv files in the directory
    qDebug() << "InjectionsPlugin::loadConfig. files are:" << csvFiles;

    // loop on the csv files one by one to creat a group for each
    for(const QString &filename : qAsConst(csvFiles))
    {
        qDebug() << "attempting to load: " << filename;
        if(filename != ".csv")  // this will cause a crash if left unchecked
        {
            InjectionGroup grp;
            grp.groupName = filename.split(".csv",QString::SkipEmptyParts,Qt::CaseInsensitive).at(0);   // get the name of the file without the .csv extention
            readInjectionsFromFile(grp.injections, directory.absolutePath() + "/" + filename);  // fill the grp object with the injections in one file
            m_injectionGroups.push_back(grp);   // push back the injections in the file as an injection group entery
        }
        else
        {
            qWarning() << ".csv file name, ignoring this file.";
        }
    }

    if (!m_injectionGroups.empty()) // if the groups are not empty (meaning a csv injections file was found)
    {
        emit injectionsLoaded(); // connected to Injections::Form::on_injectionsLoaded
    }

    return true;
}

bool InjectionsPlugin::readInjectionsFromFile(QVector<QStringList> &injections, const QString& filename)
{
    if ( filename.length() <= 0 )
    {
        qDebug() << "Can not load configuration File: " << filename;
        return false;
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Can not open configuration File: " << filename;
        return false;
    }

    while (!file.atEnd()) {
        QByteArray injectionDataLine = file.readLine();
        QString strinjectionDataLine = QString(injectionDataLine);
        /*remove newline and construc a QStringList*/
        QStringList injectionData = strinjectionDataLine.trimmed().split(",");

        /*push the injection data if it contains at least a title, appId, ctxId and srvId (4 elements)*/
        if(injectionData.count() >= 4)
        {
            injections.push_back(injectionData);
            qDebug() << "Injection read: " <<injectionData;
        }
    }
    file.close();

    return true;
}

std::vector<InjectionGroup>& InjectionsPlugin::getInjectionGroups()
{
    return m_injectionGroups;
}

bool InjectionsPlugin::saveConfig(QString /* filename */)
{
    emit unloadRequested();
    return true;
}

QStringList InjectionsPlugin::infoConfig()
{
    return QStringList();
}

QWidget* InjectionsPlugin::initViewer()
{
    form = new Injections::Form(this);
    return form;
}

bool InjectionsPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool InjectionsPlugin::initConnections(QStringList list)
{
    ecuList = new QStringList(list);
    form->setConnections(list);
    return false;
}

bool InjectionsPlugin::controlMsg(int , QDltMsg &)
{
    return false;
}

bool InjectionsPlugin::autoscrollStateChanged(bool enabled)
{
    return true;
}

void InjectionsPlugin::updateCounters(int ,int )
{

}

bool InjectionsPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname){

    qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << "\n";

    return true;
}


void InjectionsPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

void InjectionsPlugin::initMainTableView(QTableView* pTableView)
{
    Q_UNUSED(pTableView);
}

void InjectionsPlugin::configurationChanged()
{}

void InjectionsPlugin::selectedIdxMsg(int , QDltMsg &) {

}

void InjectionsPlugin::selectedIdxMsgDecoded(int , QDltMsg &){

}

void InjectionsPlugin::initFileStart(QDltFile *file){
    dltFile = file;
}

void InjectionsPlugin::initMsg(int , QDltMsg &){

}
void InjectionsPlugin::initMsgDecoded(int , QDltMsg &){

}

void InjectionsPlugin::initFileFinish(){

}

void InjectionsPlugin::updateFileStart(){

}

void InjectionsPlugin::updateMsg(int , QDltMsg &){

}

void InjectionsPlugin::updateMsgDecoded(int , QDltMsg &){

}
void InjectionsPlugin::updateFileFinish(){

}

#ifndef QT5
Q_EXPORT_PLUGIN2(dummycontrolplugin, DummyControlPlugin);
#endif
