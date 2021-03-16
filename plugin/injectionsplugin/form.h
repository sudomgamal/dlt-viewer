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

class InjectionsPlugin;

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
private:
    Ui::Form *ui;
    InjectionsPlugin *plugin;
    bool findItemInColumn(int column, QString text);

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_btnSendInjection_clicked();
    void on_tblInjections_cellDoubleClicked(int row, int column);
};

} // namespace Injections
#endif // FORM_H
