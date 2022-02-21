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
 * \file form.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QFile>

class InjectionsPlugin;

struct InjectionGroup
{
    QString groupName;
    QVector<QStringList> injections;
};

namespace Injections {
    namespace Ui {
        class Form;
    }

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(InjectionsPlugin *_plugin, QWidget *parent = 0);
    ~Form();

    void setConnections(QStringList list);
protected:
    void keyPressEvent ( QKeyEvent * event );
private:
    Ui::Form *ui;
    InjectionsPlugin *plugin;
    QFile injectionsFile;
    QAtomicInt m_addingInjections = 0;
    bool findItemInColumn(int column, QString text);

public slots:
    void on_injectionsLoaded();
    void on_unloadRequested();

    bool saveInjectionGroupToFile(InjectionGroup group);
private slots:
    void on_tblInjections_cellChanged(int row, int column);

private slots:
    void on_cmbInjGroup_editTextChanged(const QString &arg1);

private slots:
    void on_cmbInjGroup_currentIndexChanged(int index);

private slots:
    void on_btnSendInjection_clicked();
    void on_tblInjections_cellDoubleClicked(int row, int column);
    void addInjectionToTable(QStringList injection);
    bool injectionExistsInTable(QStringList injection);
    void on_btnForwardPort_clicked();
    void on_lineEditInjectionTitle_returnPressed();
    void on_lineEditApplicationId_returnPressed();
    void on_lineEditContextId_returnPressed();
    void on_lineEditServiceId_returnPressed();
    void on_lineEditData_returnPressed();
};

} // namespace Injections
#endif // FORM_H
