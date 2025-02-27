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
 * \file form.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QStandardItemModel>
#include <QTimer>
#include <QMutex>
#include <QAtomicInt>
#include <QHash>
#include <QSharedPointer>
#include "qdltcontrol.h"
#include "metric.h"
#include "modelitem.h"
#include "consumer.h"

namespace UtilizationPlotterUI {
namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    void setUser(QString text);
    void setDltControl(QDltControl* ctrl) { m_dltControl = ctrl; }
    void addEntry(double x, double y, int dltMsgIdx, Metric entryType, const QString ecu, const bool overallItem, const bool resetDetected, const int resetNumber, const QString appId = "Overall", const int pid = -1);
    void reset();

private slots:
    void updateGraph();
    void on_chkHideLowConsumers_toggled(bool checked);
    void on_cmbPlottables_customContextMenuRequested(const QPoint &pos);
    void on_chkStackGraphs_toggled(bool state);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QColor getARandomeColor(int transparency = 255);
    PointMetadata getPointMetadata(double key, double value) const;
    void handleCmbPlottablesItemChange(QStandardItem *changedItem);
    void handleEcuResetComboBoxItemChange(QStandardItem *changedItem);
    void handleCmbUsageTypeItemChange(QStandardItem *changedItem);

    QTimer *m_replotTimer;
    QStandardItemModel *m_graphItemsModel;
    QStandardItemModel *m_ecuResetListModel;
    QStandardItemModel *m_usageTypeModel;
    QHash<Metric, Qt::CheckState> m_resourceMonitorEnableMap;
    QMutex m_graphDataQMtx;
    QAtomicInt m_allowPlotting;
    QDltControl *m_dltControl;
    QHash<QString, QSharedPointer<Consumer>> m_consumersMap;
    QHash<QString, QSharedPointer<Consumer>> m_enabledConsumersMap;
    QHash<QPair<QString, int>, bool> m_ecuResetsEnableMap;
    QHash<QString, QHash<int, double>> m_ecuResetsMaxTimestamp;
    const QHash<Metric, QString> m_resourceToStringMap;
    QHash<QPair<double, double>, PointMetadata> m_pointsMetadataMap;
    bool m_stackGrpahs;
    QAtomicInt m_resizeNeeded;
    Ui::Form *ui;
};

} // namespace UtilizationPlotterUI

#endif // FORM_H
