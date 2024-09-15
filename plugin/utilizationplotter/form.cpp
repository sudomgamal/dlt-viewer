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
 * \file form.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "form.h"
#include "ui_form.h"
#include <synchapi.h>

using namespace UtilizationPlotterUI;

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    ui->lblTip->hide();
    ui->cmpPlotableItems->setModel(m_graphItemsModel);
    static auto plotterInst = ui->plotter;

    ui->plotter->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    connect(ui->plotter, &QCustomPlot::mouseMove, this, [this](QMouseEvent *ev){
                if(!plotterInst->axisRects().empty())
                {
                    if(ev->buttons() & Qt::LeftButton)
                    {
                        ui->chkAutoRedraw->setCheckState(Qt::Unchecked);
                    }
                    if(ui->lblTip->isVisible())
                    {
                        Sleep(1500);
                        ui->lblTip->hide();
                    }

                }
            });

    connect(ui->plotter, &QCustomPlot::legendClick, this, [this](QCPLegend *, QCPAbstractLegendItem *item, QMouseEvent *){
                if(!plotterInst->axisRects().empty())
                {
                    if (auto plItem = qobject_cast<QCPPlottableLegendItem*>(item))
                    {
                        plItem->plottable()->setVisible(!plItem->plottable()->visible());
                        // QString gname = plItem->plottable()->name().split(" [")[0];
                        // m_apps[gname].listItem->setCheckState(plItem->plottable()->visible()? Qt::Checked: Qt::Unchecked);
                        ui->plotter->replot(); // don't forget to replot
                    }
                }
            });

    connect(ui->plotter, &QCustomPlot::plottableClick, this, [this](QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *ev){
                if(!plotterInst->axisRects().empty())
                {
                    if(!plottable || !plottable->interface1D())
                    {
                        return;
                    }
                    double val = plottable->interface1D()->dataMainValue(dataIndex);
                    double ts = plottable->interface1D()->dataMainKey(dataIndex);
                    QString message = QString("%1 => util:%3% t:%2)").arg(plottable->name()).arg(ts).arg(val);
                    QPalette palette = ui->lblTip->palette();
                    palette.setColor(ui->lblTip->foregroundRole(), plottable->pen().color());
                    palette.setColor(ui->lblTip->backgroundRole(), QColor(255,255,0));
                    ui->lblTip->setPalette(palette);
                    ui->lblTip->setText(message);
                    ui->lblTip->adjustSize();
                    // ui->lblTip->move((ev->pos()));
                    ui->lblTip->show();
                }
            });

    // give the axes some labels:
    ui->plotter->xAxis->setLabel("TIMESTAMP [s]");
    ui->plotter->yAxis->setLabel("CPU UTILIZATION [%]");
    // set axes ranges, so we see all data:
    ui->plotter->yAxis->setRange(-5, 105);
    ui->plotter->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->plotter->legend->setFont(legendFont);
    ui->plotter->legend->setBrush(QBrush(QColor(255,255,255,230)));
    auto axisRectsList = ui->plotter->axisRects();
    for(auto* pAxisRect : axisRectsList)
    {
        pAxisRect->setRangeDrag(Qt::Orientation::Horizontal);
        pAxisRect->setRangeZoom(Qt::Orientation::Horizontal);
    }

    connect(m_graphItemsModel, &QStandardItemModel::itemChanged, this, &Form::handleListItemChange);
    connect(m_replotTimer, &QTimer::timeout, this, &Form::updateGraph);
    m_replotTimer->setInterval(50);
    m_replotTimer->start();
}

Form::~Form()
{
    delete ui;
}

void Form::reset()
{
    m_allowPlotting = 0;
    m_maxTs = 0;
    m_graphItemsModel->clear();
    m_apps.clear();

    m_graphDataQMtx.lock();
    m_graphDataQ.clear();
    m_graphDataQMtx.unlock();

    if(ui && ui->plotter && ui->plotter->graphCount() > 0)
    {
        ui->plotter->clearGraphs();
        ui->plotter->clearItems();
        ui->plotter->clearPlottables();
        ui->plotter->replot();
    }
    m_allowPlotting = 1;
}


void Form::updateGraph()
{
    if(ui->chkAutoRedraw->isChecked())
    {
        const double offset = 20;
        double min = m_maxTs - ui->spnXRange->value() - offset;
        double max = (ui->spnXRange->value() < m_maxTs) ? m_maxTs + offset: ui->spnXRange->value() + offset;
        ui->plotter->xAxis->setRange(min, max);
    }

    while(!m_graphDataQ.empty() && m_allowPlotting)
    {
        QPair<QString, QPair<double, double>> nameCoordsPair;
        m_graphDataQMtx.lock();
        if(!m_graphDataQ.empty())
        {
            nameCoordsPair = m_graphDataQ.dequeue();
        }
        m_graphDataQMtx.unlock();
        QString index = nameCoordsPair.first.split(" [")[0];

        if(m_apps[index].graphs[nameCoordsPair.first] != nullptr)
        {
            m_apps[index].graphs[nameCoordsPair.first]->addData(nameCoordsPair.second.first, nameCoordsPair.second.second);
        }
        else
        {

            m_apps[index].graphs[nameCoordsPair.first] = ui->plotter->addGraph();
            auto graph = m_apps[index].graphs[nameCoordsPair.first];
            graph->setAdaptiveSampling(true);
            graph->setLineStyle(QCPGraph::lsLine);
            graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
            graph->setName(nameCoordsPair.first);
            graph->addData(nameCoordsPair.second.first, nameCoordsPair.second.second);
            graph->setVisible(false);
            if(nameCoordsPair.first.contains("Overall"))
            {
                graph->setPen(QPen(getColor()));
                graph->setVisible(true);
            }
            else
            {
                graph->removeFromLegend();
            }
        }
    }
    ui->plotter->replot();
}

void Form::keyPressEvent(QKeyEvent *event)
{
    if(false == event->isAutoRepeat())
    {
        if(event->key() == Qt::Key_Shift)
        {
            auto axisRectsList = ui->plotter->axisRects();
            for(auto* pAxisRect : axisRectsList)
            {
                pAxisRect->setRangeDrag(Qt::Orientation::Vertical);
                pAxisRect->setRangeZoom(Qt::Orientation::Vertical);
            }
        }
    }
}
void Form::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
    {
        auto axisRectsList = ui->plotter->axisRects();
        for(auto* pAxisRect : axisRectsList)
        {
            pAxisRect->setRangeDrag(Qt::Orientation::Horizontal);
            pAxisRect->setRangeZoom(Qt::Orientation::Horizontal);
        }
    }

    if(event->key() == Qt::Key_Space)
    {
        ui->chkAutoRedraw->setCheckState(ui->chkAutoRedraw->checkState() != Qt::Checked? Qt::Checked: Qt::Unchecked);
    }
}

void Form::setUser(QString text)
{
    // ui->lineEditUser->setText(text);
}

void Form::setNice(QString text)
{
    ui->label_2->setText(text);
}

void Form::addEntry(double x, double y, const QString &graphName, const int pid, const int yMax)
{
    QString gName = QString("%1 [%2]").arg(graphName).arg(pid);
    if(!pid)
    {
        gName = graphName;
    }
    m_graphDataQMtx.lock();
    m_graphDataQ.enqueue({gName, {x, y}});
    m_graphDataQMtx.unlock();

    if(m_apps[graphName].listItem == nullptr)
    {
        m_apps[graphName].listItem = new ModelItem();
        m_apps[graphName].listItem->setText(graphName);
        m_apps[graphName].listItem->setAppsRef(&m_apps);
        if(gName == "Overall")
        {
            m_apps[graphName].listItem->setCheckState(Qt::Checked);
        }
        else
        {
            m_apps[graphName].listItem->setCheckState(Qt::Unchecked);
        }
        m_apps[graphName].listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        m_graphItemsModel->appendRow(m_apps[graphName].listItem);
        ui->cmpPlotableItems->adjustSize();
    }
    if (m_maxTs < x)
    {
        m_maxTs = x;
    }

    if (x < m_minTs)
    {
        m_minTs = x;
    }
    m_apps[graphName].maxUtilization = yMax;
}

void Form::handleListItemChange(QStandardItem * item)
{
    QString key = item->text();
    for(auto &graphPtr : m_apps[key].graphs)
    {
        if(item->checkState())
        {
            graphPtr->setPen(QPen(getColor()));
            graphPtr->setVisible(true);
            graphPtr->addToLegend();
        }
        else
        {
            graphPtr->setVisible(false);
            graphPtr->removeFromLegend();
        }
    }
    // ui->cmpPlotableItems->setCurrentIndex(item->index().row());
}


void Form::on_pushButton_clicked()
{
    std::vector<PlotDetails> plotVec;
    for (auto &app : (m_apps))
    {
        for(auto &graph:  app.graphs)
        {
            if(graph->name().startsWith("Overall"))
            {
                continue;
            }
            graph->setVisible(false);
            graph->removeFromLegend();
        }
        plotVec.push_back(app);
    }

    std::sort(plotVec.begin(), plotVec.end(), [](const PlotDetails &a, const PlotDetails &b){
                  return a.maxUtilization > b.maxUtilization;
              });

    int counter = 0;
    for (auto var = plotVec.begin(); var < plotVec.end(); ++var)
    {
        if (counter > 4)
        {
            break;
        }
        var->listItem->setCheckState(Qt::Checked);
        for(auto &graphPtr : var->graphs)
        {
            graphPtr->setPen(getColor());
            graphPtr->setVisible(true);
            graphPtr->addToLegend();
        }
        counter++;
    }
}


void Form::on_cmpPlotableItems_customContextMenuRequested(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    QAction sortUtil("Sort by Max Utilization", this);
    QAction sortAlpha("Sort Alphabetically", this);
    auto cbkSortUtil = [this]()
    {
        m_graphItemsModel->setSortRole(Qt::UserRole);
        m_graphItemsModel->sort(0, Qt::DescendingOrder);
    };


    auto cbkSortName = [this]()
    {
        m_graphItemsModel->setSortRole(Qt::DisplayRole);
        m_graphItemsModel->sort(0, Qt::AscendingOrder);
    };
    connect(&sortUtil, &QAction::triggered, this, cbkSortUtil);
    connect(&sortAlpha, &QAction::triggered, this, cbkSortName);

    contextMenu.addAction(&sortUtil);
    contextMenu.addAction(&sortAlpha);


    contextMenu.exec(ui->cmpPlotableItems->mapToGlobal(pos));
}

