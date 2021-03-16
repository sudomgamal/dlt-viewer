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

bool InjectionsPlugin::loadConfig(QString  filename )
{
    /*
    if ( filename.length() <= 0 )
    {
        // no configuration file provided, return to default configuration
//        config.setDefault();
        // return no error
        errorText = "";
        return true;
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        errorText = "Can not load configuration File: ";
        errorText.append(filename);
        return false;
    }

//    config.setDefault();
//    ft_autosave = false;
//    form->setAutoSave(config.getAutoSavePath(), ft_autosave);

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {
              if(xml.name() == QString("TAG_FLST"))
              {
//                  config.setFlstTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLDA"))
              {
//                  config.setFldaTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLFI"))
              {
//                  config.setFlfiTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLER"))
              {
//                  config.setFlerTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLAPPID"))
              {
//                  config.setFlAppIdTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLCTID"))
              {
//                  config.setFlCtIdTag( xml.readElementText() );
              }
              if(xml.name() == QString("AUTOSAVE"))
              {
//                  config.setAutoSavePath( xml.readElementText() );
//                  ft_autosave = true;
                  if (!QDir("config.getAutoSavePath()").exists())
                  {
                    if ( false == QDir().mkpath("config.getAutoSavePath()") )
                    {
                     if ( dltControl->silentmode == true )
                      {
                      qDebug() << "ERROR creating autosave folder" << "config.getAutoSavePath()";
                      }
                     else
                      {
                       QMessageBox::warning(0, QString("ERROR creating autosave folder"), "config.getAutoSavePath()");
                      }
                    }
                  }
//                form->setAutoSave(config.getAutoSavePath(), true);
              }

          }
    }
    if (xml.hasError())
    {
      if ( dltControl->silentmode == true )
      {
       qDebug() << "dumm" << QString("XML Parser error %1 at line %2").arg(xml.errorString()).arg(xml.lineNumber());
      }
      else
      {
        QMessageBox::warning(0, QString("XML Parser error"), xml.errorString());
      }
    }

    file.close();
    */

    return true;
}

bool InjectionsPlugin::saveConfig(QString /* filename */)
{
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
