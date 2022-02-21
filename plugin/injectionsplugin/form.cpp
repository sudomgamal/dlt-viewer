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
 * \file form.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "form.h"
#include "ui_form.h"
#include "injectionsplugin.h"
#include <QtGui>
#include <qfiledialog.h>

using namespace Injections;

Form::Form(InjectionsPlugin *_plugin,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form),
    injectionsFile("injections.csv")
{
    ui->setupUi(this);
    plugin = _plugin;
    connect(plugin, &InjectionsPlugin::injectionsLoaded, this, &Injections::Form::on_injectionsLoaded);
    connect(plugin, &InjectionsPlugin::unloadRequested, this, &Form::on_unloadRequested);

    QTableWidgetItem *tblItem = new QTableWidgetItem("");
    ui->tblInjections->setHorizontalHeaderItem(0, tblItem);

    tblItem = new QTableWidgetItem("Title");
    ui->tblInjections->setHorizontalHeaderItem(1, tblItem);

    tblItem = new QTableWidgetItem("AppId");
    ui->tblInjections->setHorizontalHeaderItem(2, tblItem);

    tblItem = new QTableWidgetItem("CtxId");
    ui->tblInjections->setHorizontalHeaderItem(3, tblItem);

    tblItem = new QTableWidgetItem("SrvId");
    ui->tblInjections->setHorizontalHeaderItem(4, tblItem);

    tblItem = new QTableWidgetItem("data");
    ui->tblInjections->setHorizontalHeaderItem(5, tblItem);

    ui->tblInjections->resizeColumnsToContents();
    plugin->loadConfig(injectionsFile.fileName());
}

Form::~Form()
{
    delete ui;
    on_unloadRequested();
}

void Form::setConnections(QStringList list)
{
    ui->comboBoxConnections->clear();
    ui->comboBoxConnections->insertItems(0,list);
}

bool Form::findItemInColumn(int column, QString text)
{
    int nRows = ui->tblInjections->rowCount();

    for (int i = 0; i< nRows; ++i)
    {
        if(ui->tblInjections->item(i, column) && ui->tblInjections->item(i, column)->text() == text)
        {
            return true;
        }
    }

    return false;
}

void Injections::Form::on_btnSendInjection_clicked()
{
    QString injTitle = ui->lineEditInjectionTitle->text();
    QString appId = ui->lineEditApplicationId->text();
    QString ctxId = ui->lineEditContextId->text();
    QString srvId = ui->lineEditServiceId->text();
    QString payload = ui->lineEditData->text();

    if(plugin->dltControl)
    {
        if (payload.isEmpty())
        {
            payload.append('\0');
        }
        plugin->dltControl->sendInjection(ui->comboBoxConnections->currentIndex(),
                                          appId, ctxId, srvId.toInt(),payload.toLatin1());
    }
    else
    {
        qDebug()<<"ERROR: dltControl plugin is not loaded. Can not send injection" ;
    }

    QStringList injection;
    injection << injTitle << appId << ctxId << srvId;
    if (payload != '\0')
    {
        injection << payload;
    }

    if (injectionExistsInTable(injection))
    {
        qDebug() <<"Injection already exists. Maybe with a different title";
        return;
    }
    else if(appId.isEmpty() || ctxId.isEmpty() || (srvId.toInt() < 0xFFF || srvId.toULongLong() > 0xFFFFFFFF))
    {
        qDebug() <<"Invalid injection. Not adding to the list";
    }
    else
    {
        if (plugin->getInjectionGroups().size() > ui->cmbInjGroup->currentIndex())
        {
            plugin->getInjectionGroups().at(ui->cmbInjGroup->currentIndex()).injections.push_back(injection);
        }
        addInjectionToTable(injection);
    }
}

void Injections::Form::on_tblInjections_cellDoubleClicked(int row, int column)
{
    if (column == 0)
    {
        if (nullptr == ui->tblInjections->item(row, 2))
        {
            qDebug() << "Invalid item: (" << row <<", 2)";
            return;
        }

        QString payload = "";
        if(ui->tblInjections->item(row, 5))
        {
            payload = ui->tblInjections->item(row, 5)->text();
            if (payload.isEmpty())
            {
                payload.append('\0');
            }
        }
        else
        {
            payload.append('\0');
        }

        qDebug() << "payload: " << payload;

        if(plugin->dltControl)
        {
            plugin->dltControl->sendInjection(ui->comboBoxConnections->currentIndex(),
                                              ui->tblInjections->item(row, 2)->text(),
                                              ui->tblInjections->item(row, 3)->text(),
                                              ui->tblInjections->item(row, 4)->text().toInt(),
                                              payload.toLatin1());
        }
    }
}

void Injections::Form::addInjectionToTable(QStringList injection)
{
    m_addingInjections = 1;
    ui->tblInjections->setSortingEnabled(false);

    ui->tblInjections->setRowCount(ui->tblInjections->rowCount()+1);

    QTableWidgetItem *tblItem = new QTableWidgetItem("SEND");
    tblItem->setBackground(Qt::darkGreen);
    tblItem->setForeground(Qt::white);
    tblItem->setFlags(tblItem->flags() ^ Qt::ItemIsEditable);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 0, tblItem);

    qDebug()<< "adding injection: " << injection.join(",");
    for (int inj = 0; inj < injection.count() ; ++inj)
    {
        tblItem = new QTableWidgetItem(injection[inj]);
        ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, inj+1, tblItem);
    }

    ui->tblInjections->setSortingEnabled(true);
    ui->tblInjections->resizeColumnsToContents();
    m_addingInjections = 0;
}

bool Injections::Form::injectionExistsInTable(QStringList injection)
{
    bool result = true;
    int nRows = ui->tblInjections->rowCount();

    if(nRows == 0)
        return false;

    for (int i = 0; i< nRows; ++i)
    {
        for (int inj = 0; inj < injection.count() ; ++inj)
        {
            if(ui->tblInjections->item(i, inj+1))
            {
                if(ui->tblInjections->item(i, inj+1)->text() == injection[inj])
                {
                    result = true;
                }
                else
                {
                    result = false;
                    break;
                }
            }
            else
            {
                result = false;
                break;
            }
        }

        if(result == true)
        {
            return result;
        }
    }

    return result;
}

void Injections::Form::on_injectionsLoaded()
{
    ui->cmbInjGroup->clear();
    for (const auto &group : plugin->getInjectionGroups())
    {
        ui->cmbInjGroup->addItem(group.groupName);
    }
}

void Injections::Form::on_unloadRequested()
{
    for(auto &&grp : plugin->getInjectionGroups())
    {
        saveInjectionGroupToFile(grp);
    }
}

bool Injections::Form::saveInjectionGroupToFile(InjectionGroup group)
{
    QFile file(QCoreApplication::applicationDirPath() + "/injections/" + group.groupName + ".csv");
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream injectionsFileStream(&file);

        int nRows = group.injections.size();

        for (int r = 0; r< nRows; ++r)
        {
            int nCols = group.injections[r].size();
            for (int c = 0; c< nCols; ++c)
            {
                injectionsFileStream << group.injections[r].at(c);

                if (c < (nCols-1))
                {
                    injectionsFileStream << ",";
                }
            }

            injectionsFileStream << "\n";
        }

        qDebug() << "Injections stored";
    }

    injectionsFile.close();

    return true;
}

void Injections::Form::on_btnForwardPort_clicked()
{
#ifdef Q_OS_WIN
    QString program("cmd.exe");
    QStringList parameters;
    parameters << " /k adb wait-for-device & adb forward tcp:3490 tcp:3490";
    QProcess::startDetached(program, parameters);
#elif Q_OS_LINUX
    QString program("/bin/sh");
    QStringList parameters;
    parameters << " -c" << "adb " << "wait-for-device" << " & adb" <<
                  "forward"<< "tcp:3490" << "tcp:3490";
    QProcess::startDetached(program, parameters);
#endif
}

void Injections::Form::on_lineEditInjectionTitle_returnPressed()
{
    on_btnSendInjection_clicked();
}

void Injections::Form::on_lineEditApplicationId_returnPressed()
{
    on_btnSendInjection_clicked();
}

void Injections::Form::on_lineEditContextId_returnPressed()
{
    on_btnSendInjection_clicked();
}

void Injections::Form::on_lineEditServiceId_returnPressed()
{
    on_btnSendInjection_clicked();
}

void Injections::Form::on_lineEditData_returnPressed()
{
    on_btnSendInjection_clicked();
}

void Injections::Form::keyPressEvent ( QKeyEvent * event )
{
    if(event->key() == Qt::Key_Delete)
    {
        if(ui->tblInjections->hasFocus())
        {
            auto list = ui->tblInjections->selectionModel()->selectedRows(1);
            for (auto item : qAsConst(list))
            {
                qDebug() << "Removing " << item.row();
                ui->tblInjections->model()->removeRow(item.row());

                if (plugin->getInjectionGroups().size() > ui->cmbInjGroup->currentIndex()
                        && item.row() < plugin->getInjectionGroups().at(ui->cmbInjGroup->currentIndex()).injections.size())
                {
                    plugin->getInjectionGroups().at(ui->cmbInjGroup->currentIndex()).injections.remove(item.row());
                }
            }
        }
    }
}

void Form::on_cmbInjGroup_currentIndexChanged(int index)
{

    qDebug() << "index:" << index << "plugin->getInjectionGroups().size(): " << plugin->getInjectionGroups().size();
    if (index>=0 && index < plugin->getInjectionGroups().size())
    {
        ui->tblInjections->clearContents();
        ui->tblInjections->setRowCount(0);
        for (auto &injection : plugin->getInjectionGroups().at(index).injections)
        {
            addInjectionToTable(injection);
        }
    }
}


void Form::on_cmbInjGroup_editTextChanged(const QString &arg1)
{
    int index = ui->cmbInjGroup->currentIndex();
    qDebug() << "index:" << index << ",plugin->getInjectionGroups().size(): " << plugin->getInjectionGroups().size() <<
                "arg1:" << arg1;

    if (index == plugin->getInjectionGroups().size())
    {
        InjectionGroup newGroup;
        newGroup.groupName = arg1;
        newGroup.injections.clear();
        saveInjectionGroupToFile(newGroup);
        plugin->loadConfig("");
    }
}


void Form::on_tblInjections_cellChanged(int row, int column)
{
    qDebug() << "row changed:" << row << "m_addingInjections:" << m_addingInjections;
    if(m_addingInjections == 0 && plugin->getInjectionGroups().size() > ui->cmbInjGroup->currentIndex())
    {
        auto &injections = plugin->getInjectionGroups().at(ui->cmbInjGroup->currentIndex()).injections;
        qDebug() << "injections.size():" << injections.size() << "ui->cmbInjGroup->currentIndex():" << ui->cmbInjGroup->currentIndex();
        if(row < injections.size())
        {
            QStringList &inj = injections[row];
            qDebug() << "inj.size():" << inj.size() << "m_addingInjections:" << m_addingInjections;
            inj.clear();
            for(auto col = 1 ; col < ui->tblInjections->columnCount(); ++col)
            {
                if(ui->tblInjections->item(row, col) && ui->tblInjections->item(row, col)->text().size())
                {
                    inj << ui->tblInjections->item(row, col)->text();
                }
            }
        }
    }
}

