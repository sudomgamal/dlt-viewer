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
#include <QTreeWidgetItem>
#include <qstandarditemmodel.h>
#include <qtimer.h>
#include "plugininterface.h"
#include "qcustomplot.h"

namespace UtilizationPlotterUI {
namespace Ui {
    class Form;
}
class ModelItem;

struct PlotDetails
{
    int maxUtilization = 0;
    QMap<QString, QCPGraph *> graphs;
    ModelItem *listItem = nullptr;
};

class ModelItem: public QStandardItem
{
public:
    void setAppsRef(const QMap<QString, PlotDetails> *apps)
    {
        m_itemAppsRef = apps;
    }
protected:
    bool operator<(const QStandardItem &other) const override
    {

        auto leftData = this->data(Qt::DisplayRole).value<QString>();
        auto rightData = other.data(Qt::DisplayRole).value<QString>();
        if(this->model()->sortRole() == Qt::DisplayRole)
        {
            return (leftData.compare(rightData, Qt::CaseInsensitive) < 0);
        }

        if((m_itemAppsRef->find(leftData) != m_itemAppsRef->end()) &&
            (m_itemAppsRef->find(rightData) != m_itemAppsRef->end()))
        {
            return m_itemAppsRef->value(leftData).maxUtilization <
                   m_itemAppsRef->value(rightData).maxUtilization;
        }
        else
        {
            return false;
        }
    }

    const QMap<QString, PlotDetails> *m_itemAppsRef;
};



class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    void setUser(QString text);
    void setMainTableView(QTableView* tableView) {m_mainTableView = tableView;}
    void setNice(QString text);
    void addEntry(double x, double y, const QString &name = "Overall", const int pid = 0, const int yMax = 0);

    void reset();

private slots:
    void updateGraph();
    void on_pushButton_clicked();

    void on_cmpPlotableItems_customContextMenuRequested(const QPoint &pos);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QColor getColor()
    {
        static const std::vector<QColor> sColors
            {
                QColor(255, 0, 0),     // Bright Red
                QColor(0, 128, 0),     // Dark Green
                QColor(0, 0, 255),     // Bright Blue
                QColor(255, 165, 0),   // Orange
                QColor(128, 0, 128),   // Purple
                QColor(0, 255, 255),   // Cyan
                QColor(139, 69, 19),   // Saddle Brown
                QColor(255, 20, 147),  // Deep Pink
                QColor(255, 140, 0),   // Dark Orange
                QColor(75, 0, 130),    // Indigo
                QColor(0, 206, 209),   // Dark Turquoise
                QColor(34, 139, 34),   // Forest Green
                QColor(210, 105, 30),  // Chocolate
                QColor(148, 0, 211),   // Dark Violet
                QColor(0, 0, 139),     // Dark Blue
                QColor(255, 69, 0),    // Red-Orange
                QColor(0, 255, 127),   // Spring Green
                QColor(112, 128, 144), // Slate Gray
                QColor(0, 191, 255),   // Deep Sky Blue
                QColor(220, 20, 60),   // Crimson
                QColor(0, 128, 128),   // Teal
            };
        static const int sColorsSize = sColors.size();
        static std::atomic<int> sColorsCounter(QRandomGenerator::securelySeeded().bounded(sColorsSize));
        return sColors[sColorsCounter++ % sColorsSize];
    }
    double m_maxTs = 10;
    double m_minTs = 10;
    QTimer *m_replotTimer = new QTimer(this);
    QStandardItemModel *m_graphItemsModel = new QStandardItemModel(0,1);
    void handleListItemChange(QStandardItem * item);
    QQueue<QPair<QString, QPair<double,double>>> m_graphDataQ;
    QMutex m_graphDataQMtx;
    QAtomicInt m_allowPlotting = 0;
    QTableView* m_mainTableView = nullptr;
    QMap<QString, PlotDetails> m_apps;
    Ui::Form *ui;
};

} //namespaceUtilizationPlotterUI

#endif // FORM_H
