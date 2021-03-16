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

#include <qfiledialog.h>

using namespace Injections;

Form::Form(InjectionsPlugin *_plugin,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    plugin = _plugin;

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
}

Form::~Form()
{
    delete ui;
}

void Form::setConnections(QStringList list)
{
    ui->comboBoxConnections->clear();
    ui->comboBoxConnections->insertItems(0,list);
}

void Form::on_pushButtonConnect_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->connectEcu(ui->comboBoxConnections->currentIndex());
}

void Form::on_pushButtonDisconnect_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->disconnectEcu(ui->comboBoxConnections->currentIndex());
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
        qDebug()<<"ERROR: dltControl plugin is no loaded. Can not send injection" ;
    }

    if (findItemInColumn(2, appId) && findItemInColumn(3, ctxId)
            && findItemInColumn(4, srvId) && findItemInColumn(5, payload))
    {
        qDebug() <<"Injection already exists. Maybe with a different title";
        return;
    }

    ui->tblInjections->setSortingEnabled(false);

    ui->tblInjections->setRowCount(ui->tblInjections->rowCount()+1);

    QTableWidgetItem *tblItem = new QTableWidgetItem("SEND");
    tblItem->setBackground(Qt::darkGray);
    tblItem->setForeground(Qt::white);
    tblItem->setFlags(tblItem->flags() ^ Qt::ItemIsEditable);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 0, tblItem);

    tblItem = new QTableWidgetItem(injTitle);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 1, tblItem);

    tblItem = new QTableWidgetItem(appId);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 2, tblItem);

    tblItem = new QTableWidgetItem(ctxId);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 3, tblItem);

    tblItem = new QTableWidgetItem(srvId);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 4, tblItem);

    tblItem = new QTableWidgetItem(payload);
    ui->tblInjections->setItem(ui->tblInjections->rowCount()-1, 5, tblItem);

    ui->tblInjections->setSortingEnabled(true);
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

        qDebug() << ui->tblInjections->item(row, 5)->text();
        QString payload = ui->tblInjections->item(row, 5)->text();
        if(plugin->dltControl)
        {
            if (payload.isEmpty())
            {
                payload.append('\0');
            }
            plugin->dltControl->sendInjection(ui->comboBoxConnections->currentIndex(),
                                              ui->tblInjections->item(row, 2)->text(),
                                              ui->tblInjections->item(row, 3)->text(),
                                              ui->tblInjections->item(row, 4)->text().toInt(),
                                              payload.toLatin1());
        }
    }
}
