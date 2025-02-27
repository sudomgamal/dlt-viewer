#include "form.h"
#include "ui_form.h"

using namespace UtilizationPlotterUI;

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form),
    m_resourceToStringMap({{Metric::RssMemory, "RSS"},
    {Metric::SystemCpu, "S.CPU"},
    {Metric::TotalCpu, "CPU"},
    {Metric::VirtualMemory, "VIRT"},
    {Metric::UserCpu, "U.CPU"},
    {Metric::TotalMemory, "MEM%"},})
{
    m_replotTimer = new QTimer(this);
    m_graphItemsModel = new QStandardItemModel(0,1);
    m_ecuResetListModel = new QStandardItemModel(0,1);
    m_usageTypeModel = new QStandardItemModel(0,1);

    ui->setupUi(this);
    ui->chkHideLowConsumers->hide();
    // ui->lblTip->hide();
    // ui->cmbPlottables->setModel(m_graphItemsModel);
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_graphItemsModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0);
    ui->cmbPlottables->setModel(proxyModel);

    auto completer = new QCompleter(proxyModel, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setFilterMode(Qt::MatchContains); // Ensure the filter matches any part of the string
    ui->cmbPlottables->setCompleter(completer);

    QObject::connect(completer, SIGNAL(activated(const QString &)), this, SLOT(onCompleterActivated(const QString &)));
    connect(ui->cmbPlottables->lineEdit(), &QLineEdit::textEdited, this, [proxyModel, this](const QString &text){
        // qDebug() << text;
        proxyModel->setFilterFixedString(text);
        ui->cmbPlottables->setCurrentIndex(-1);
        ui->cmbPlottables->setEditText(text);
        // ui->cmbPlottables->showPopup();
    });

    ui->cmbEcuResetSelector->setModel(m_ecuResetListModel);
    ui->cmbUsageType->setModel(m_usageTypeModel);
    connect(m_usageTypeModel, &QStandardItemModel::itemChanged, this, &Form::handleCmbUsageTypeItemChange);
    ui->cmbUsageType->view()->setUpdatesEnabled(true);
    auto addUsageTypeToModel = [this](Metric type, const QString &text)
    {
        QStandardItem *usageTypeItem = new QStandardItem();
        usageTypeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        usageTypeItem->setText(text);
        usageTypeItem->setData(QVariant::fromValue(static_cast<int>(type))); // Store the enum value as item data
        m_resourceMonitorEnableMap[type] = type == Metric::TotalCpu || type == Metric::TotalMemory ? Qt::Checked: Qt::Unchecked;
        m_usageTypeModel->appendRow(usageTypeItem);
        usageTypeItem->setCheckState(type == Metric::TotalCpu || type == Metric::TotalMemory ? Qt::Checked: Qt::Unchecked);
    };
    addUsageTypeToModel(Metric::TotalCpu, "Total CPU");
    addUsageTypeToModel(Metric::SystemCpu, "System CPU");
    addUsageTypeToModel(Metric::UserCpu, "User CPU");
    addUsageTypeToModel(Metric::TotalMemory, "Total Memory[%]");
    addUsageTypeToModel(Metric::RssMemory, "RSS Memory[MB]");
    addUsageTypeToModel(Metric::VirtualMemory, "Virtual Memory[MB]");
    ui->cmbUsageType->adjustSize();
    ui->cmbUsageType->resize(ui->cmbUsageType->width() + ui->cmbUsageType->iconSize().width(), ui->cmbUsageType->height());
    ModelItem::consumersMapPtr = &m_consumersMap;
    ModelItem::ecuResetsEnableMap = &m_ecuResetsEnableMap;
;
    static auto plotterInst = ui->plotter;

#if _WIN32
    //Windows 11 doesn't show QCumboBox well in fusion style.
    ui->cmbPlottables->setStyle(QStyleFactory::create("windowsvista"));
    ui->cmbPlottables->setStyleSheet("combobox-popup: 0;");
#endif

    ui->plotter->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    connect(ui->plotter, &QCustomPlot::mouseMove, this, [this](QMouseEvent *ev)
            {
                if(!plotterInst->axisRects().empty())
                {
                    if(ev->buttons() & Qt::LeftButton)
                    {
                        ui->chkAutoRedraw->setCheckState(Qt::Unchecked);
                    }
                }
            });

    connect(ui->plotter, &QCustomPlot::legendClick, this, [this](QCPLegend *, QCPAbstractLegendItem *item, QMouseEvent *)
            {
                if(!plotterInst->axisRects().empty())
                {
                    if (auto plItem = qobject_cast<QCPPlottableLegendItem*>(item))
                    {
                        plItem->plottable()->setVisible(!plItem->plottable()->visible());
                        ui->plotter->replot(); // don't forget to replot
                    }
                }
            });

    connect(ui->plotter, &QCustomPlot::plottableClick, this, [this](QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *ev)
            {
                if(plotterInst->axisRects().empty() || !plottable || !plottable->interface1D())
                {
                    return;
                }
                double val = plottable->interface1D()->dataMainValue(dataIndex);
                double ts = plottable->interface1D()->dataMainKey(dataIndex);
                auto md = getPointMetadata(ts, val);
                UsageGraph* graph = qobject_cast<UsageGraph*>(plottable);
                GraphDataMapKey mapKey = {md.powerCycle, md.metric};
                const auto &gData = m_consumersMap[graph->plottableModeltem->consumersMapKey]->graphDataMap[mapKey];
                if(ev->button() == Qt::LeftButton)
                {
                    if (graph)
                    {
                        auto dataContainer = graph->data();
                        // Check if the dataIndex is valid
                        if (dataIndex >= 0 && dataIndex < dataContainer->size())
                        {
                            qDebug() << "Data point at index" << dataIndex << ": (" << ts
                                     << "," << val << ", type:" << md.metric << "(" << graph->metric << ", idx:" << md.dltIdx << ")";
                            if(m_dltControl && md.dltIdx)
                            {
                                m_dltControl->jumpToMsg(md.dltIdx);
                            }
                        }
                        else
                        {
                            qDebug() << "Invalid dataIndex:" << dataIndex;
                        }
                    }
                    else
                    {
                        qDebug() << "Plottable is not a QCPGraph.";
                    }
                    QString message = QString("%1\nTimestamp: %2\nUsage: %3\nAverage: %4\nMaximum: %5").
                                      arg(plottable->name()).arg(ts).arg(val).arg(gData.resourceStats.average).arg(gData.resourceStats.max);
                    QToolTip::showText(ev->globalPos(), message);
                    qDebug() << message;
                }
                if(ev->button() == Qt::RightButton)
                {
                    QMenu contextMenu(tr("Context menu"), this);
                    QAction actSortCpuUtil("Show highest consumers at this point", this);
                    auto cbkSortCpuUtil = [this, ts, graph]()
                    {
                        QMap<double, ModelItem*> consumersAroundTimestamp;
                        // qDebug() << __func__ << "Starting cbkSortCpuUtil with timestamp:" << ts;

                        m_graphDataQMtx.lock();
                        // Loop through all consumers in the map
                        for(const auto &c : qAsConst(m_consumersMap))
                        {
                            // Skip if the consumer is null or does not match the ECU of the selected graph
                            if(c.isNull() || c->plottableModeltem == nullptr || graph->plottableModeltem == c->plottableModeltem ||
                                m_consumersMap[graph->plottableModeltem->consumersMapKey]->ecu != c->ecu)
                            {
                                // qWarning() << __func__ << "Skipping consumer due to null pointer or ECU mismatch:" << c->ecu;
                                continue;
                            }

                            // Uncheck other consumers that are not overall consumers
                            if(graph->plottableModeltem != c->plottableModeltem && !c->plottableModeltem->isOverallConsumer)
                            {
                                // qDebug() << __func__ << "Unchecking consumer:" << c->plottableModeltem->text();
                                m_graphDataQMtx.unlock();
                                c->plottableModeltem->setCheckState(Qt::Unchecked);
                                m_graphDataQMtx.lock();
                            }

                            // Loop through all graph keys in the consumer's graph data map
                            for(const auto &graphKey : c->graphDataMap.keys())
                            {
                                // Skip if the graph data points are null, empty, or do not match the power cycle of the selected graph
                                if(c->graphDataMap[graphKey].points == nullptr ||
                                    c->graphDataMap[graphKey].points->isEmpty() || graphKey.powerCycle != graph->powerCycle)
                                {
                                    // qWarning() << __func__ << "Skipping graph key due to null points, empty points, or power cycle mismatch:" << graphKey.powerCycle;
                                    continue;
                                }

                                // Loop through all data points in the graph data container
                                for (auto const &p : *c->graphDataMap[graphKey].points)
                                {
                                    // Find the nearest timestamp to the selected point's timestamp
                                    if(p.key > ts)
                                    {
                                        // Add the y value and corresponding model item to the sorted map
                                        // qDebug() << __func__ << "Adding consumer to sorted map:" << c->plottableModeltem->text() << "with value:" << p.value;
                                        consumersAroundTimestamp.insert(p.value, c->plottableModeltem);
                                        break;
                                    }
                                }
                            }
                        }
                        // Perform custom function on the first 5 elements in the sorted map (from highest to lowest)
                        int counter = 0;
                        // qDebug() << __func__ << "showing consumersss:" << consumersAroundTimestamp.size();
                         QMutableMapIterator<double, ModelItem*> consumerItem(consumersAroundTimestamp);
                        consumerItem.toBack();
                        while(consumerItem.hasPrevious())
                        {
                            consumerItem.previous();
                            // qDebug() << __func__ << "showing consumer:" << consumerItem.key() << consumerItem.value()->text();
                            m_graphDataQMtx.unlock();
                            consumerItem.previous().value()->setCheckState(Qt::Checked);
                            m_graphDataQMtx.lock();
                            if(++counter == 5)
                            {
                                break;
                            }
                        }
                        m_graphDataQMtx.unlock();
                    };

                    connect(&actSortCpuUtil, &QAction::triggered, this, cbkSortCpuUtil);
                    contextMenu.addAction(&actSortCpuUtil);
                    contextMenu.exec(ui->plotter->mapToGlobal(ev->pos()));
                }
            });

    // give the axes some labels:
    ui->plotter->xAxis->setLabel("TIMESTAMP [s]");
    ui->plotter->yAxis->setLabel("Usage");
    // set axes ranges, so we see all data:
    ui->plotter->yAxis->setRange(-5, 105);
    ui->plotter->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->plotter->legend->setFont(legendFont);
    ui->plotter->legend->setBrush(QBrush(QColor(255,255,255,230)));
    auto axisRectsList = ui->plotter->axisRects();
    for(auto* pAxisRect : std::as_const(axisRectsList))
    {
        pAxisRect->setRangeDrag(Qt::Orientation::Horizontal);
        pAxisRect->setRangeZoom(Qt::Orientation::Horizontal);
    }

    connect(m_graphItemsModel, &QStandardItemModel::itemChanged, this, &Form::handleCmbPlottablesItemChange);
    connect(m_ecuResetListModel, &QStandardItemModel::itemChanged, this, &Form::handleEcuResetComboBoxItemChange);
    connect(m_ecuResetListModel, &QStandardItemModel::itemChanged, this, &Form::handleEcuResetComboBoxItemChange);
    connect(ui->chkStackGraphs, &QCheckBox::toggled, this, &Form::on_chkStackGraphs_toggled);
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
    m_graphItemsModel->clear();
    m_ecuResetListModel->clear();
    m_consumersMap.clear();
    m_ecuResetsEnableMap.clear();
    m_pointsMetadataMap.clear();

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
        double lowerBound = 0;
        double upperBound = 0;
        for(const auto &ecu: m_ecuResetsMaxTimestamp.keys())
        {
            for(const auto &rst: m_ecuResetsMaxTimestamp[ecu].keys())
            {
                if(m_ecuResetsEnableMap.contains({ecu, rst}) && m_ecuResetsEnableMap[{ecu, rst}] == true)
                {
                    if(upperBound < m_ecuResetsMaxTimestamp[ecu][rst])
                    {
                        upperBound = m_ecuResetsMaxTimestamp[ecu][rst];
                    }
                }
            }
        }

        lowerBound = upperBound - ui->spnXRange->value() - offset;
        upperBound = (ui->spnXRange->value() < upperBound) ? upperBound + offset: ui->spnXRange->value() + offset;
        // qDebug() << __FUNCTION__ << min << max;
        ui->plotter->xAxis->setRange(lowerBound, upperBound);
    }

    if(m_resizeNeeded)
    {
        for(auto &combobox: {/*ui->cmbPlottables,*/ ui->cmbEcuResetSelector, ui->cmbUsageType})
        {
            auto cw = combobox->width();
            combobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            combobox->adjustSize();
            combobox->resize(combobox->width() +  combobox->iconSize().width(), combobox->height());
            if(cw < combobox->width())
            {
                combobox->setMaximumWidth(combobox->width());
                combobox->setMinimumWidth(combobox->width());
            }
            combobox->parentWidget()->updateGeometry();
            combobox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);

        }
        m_resizeNeeded = 0;
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
            for(auto* pAxisRect : std::as_const(axisRectsList))
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
        for(auto* pAxisRect : std::as_const(axisRectsList))
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

void Form::addEntry(double x, double y, int dltMsgIdx, Metric metric, const QString ecu, const bool isOverall, const bool resetDetected, const int powerCycle, const QString consumerName, const int pid)
{
    m_graphDataQMtx.lock();
    //make sure the list contains reset data upto the reset number
    QString consumerKey = QString("%1_%2").arg(ecu, consumerName);
    GraphDataMapKey graphKey({powerCycle, metric});
    if(m_consumersMap.value(consumerKey, nullptr) == nullptr)
    {
        m_consumersMap[consumerKey] = QSharedPointer<Consumer>::create();
    }
    if(resetDetected)
    {
        auto state = m_ecuResetsEnableMap.isEmpty() ? Qt::Checked: Qt::Unchecked;
        QStandardItem *ecuResetItem = new QStandardItem();
        ecuResetItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ecuResetItem->setText(QString("%1:%2").arg(ecu, QString::number(powerCycle)));
        ecuResetItem->setData(QVariant::fromValue(QPair<QString, int>(ecu, powerCycle)));
        m_ecuResetListModel->appendRow(ecuResetItem);
        ecuResetItem->setCheckState(state); //first ecu. set it as checked.
        m_ecuResetsEnableMap[{ecu, powerCycle}] = state;
        m_ecuResetsMaxTimestamp[ecu][powerCycle] = 0;
        qDebug() << "resetDetected:" << ecu << powerCycle << state << x << y  << metric << isOverall << consumerName;
    }

    m_consumersMap[consumerKey]->ecu = ecu;
    m_consumersMap[consumerKey]->consumerName = consumerName;
    if(m_consumersMap[consumerKey]->graphDataMap[graphKey].points == nullptr)
    {
        // qDebug() << "new QCPGraphData for" << consumerKey << entryType << pid << resetNumber << m_consumersMap[consumerKey]->graphDataMap.size();
        m_consumersMap[consumerKey]->graphDataMap[graphKey].points = QSharedPointer<QCPDataContainer<QCPGraphData>>::create();
    }
    m_consumersMap[consumerKey]->graphDataMap[graphKey].points->add(QCPGraphData(x ,y));
    m_consumersMap[consumerKey]->graphDataMap[graphKey].pid = pid;
    m_pointsMetadataMap[{x, y}] = {metric, powerCycle, pid, dltMsgIdx};

    if(m_consumersMap[consumerKey]->plottableModeltem == nullptr)
    {
        m_consumersMap[consumerKey]->plottableModeltem = new ModelItem();
        m_consumersMap[consumerKey]->plottableModeltem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        m_consumersMap[consumerKey]->plottableModeltem->setCheckState(Qt::Unchecked);
        m_consumersMap[consumerKey]->plottableModeltem->isOverallConsumer = isOverall;
        m_consumersMap[consumerKey]->plottableModeltem->setText(consumerKey);
        m_consumersMap[consumerKey]->plottableModeltem->consumersMapKey = consumerKey;
        m_graphItemsModel->appendRow(m_consumersMap[consumerKey]->plottableModeltem);
        if(isOverall && (m_resourceMonitorEnableMap[metric] == Qt::Checked))
        {
            m_consumersMap[consumerKey]->plottableModeltem->setCheckState(Qt::Checked);
            m_graphDataQMtx.unlock();
            emit m_graphItemsModel->itemChanged(m_consumersMap[consumerKey]->plottableModeltem);
            m_graphDataQMtx.lock();
        }
        // qDebug() << "new entry:" << (ecu + "_" + name) << isOverall << m_usageTypeCheckState[entryType];
    }

    m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.sum += y;
    m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.average = m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.sum /
                                                                                m_consumersMap[consumerKey]->graphDataMap[graphKey].points->size();
    if(y > m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.max)
    {
        m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.max = y;
    }

    if (m_ecuResetsMaxTimestamp[ecu][powerCycle] < x)
    {
        m_ecuResetsMaxTimestamp[ecu][powerCycle] = x;
    }

    m_graphDataQMtx.unlock();

    if(TotalCpu == metric)
    {
        m_graphItemsModel->blockSignals(true);
        m_ecuResetListModel->blockSignals(true);
        m_consumersMap[consumerKey]->plottableModeltem->setText(
            QString("%1 (max:%2 avg:%3)").arg(consumerKey)
                .arg(m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.max, 0, 'f', 2)
                .arg(m_consumersMap[consumerKey]->graphDataMap[graphKey].resourceStats.average, 0, 'f', 2));
        // ui->cmbPlottables->adjustSize();

        m_graphItemsModel->blockSignals(false);
        m_ecuResetListModel->blockSignals(false);
    }

    m_resizeNeeded = 1;
    // QListView* view = qobject_cast<QListView *>(ui->cmbPlottables->view());
    // if(view && ui->chkHideLowConsumers->isChecked() && m_entityPlotDetailsMap[plottablesMapKey]->plottableModeltem->maxTotalCpuUsage <= 0.05)
    // {
    //     view->setRowHidden(m_entityPlotDetailsMap[plottablesMapKey]->plottableModeltem->row(), true);
    // }
}

void Form::handleCmbPlottablesItemChange(QStandardItem *changedItem)
{
    auto item = (ModelItem*) changedItem;
    if(m_consumersMap.empty() || !m_consumersMap.contains(item->consumersMapKey) || m_consumersMap.value(item->consumersMapKey, nullptr) == nullptr)
    {
        return;
    }
    // qDebug() << __FUNCTION__ << changedItem->text() << "chenged:" << changedItem->checkState() <<
    //     m_entityPlotDetailsMap[item->plottablesMapKey]->ecu <<
    //     m_entityPlotDetailsMap[item->plottablesMapKey]->processName << m_entityPlotDetailsMap[item->plottablesMapKey]->graphPidMap.size() <<
    //     m_entityPlotDetailsMap[item->plottablesMapKey]->graphList.size();

    m_graphDataQMtx.lock();

    //clear everything anyway. graphs will be added later if neccessary
    for(auto &g: m_consumersMap.value(item->consumersMapKey)->plottedGraphs)
    {
        ui->plotter->removeGraph(g);
    }
    m_consumersMap.value(item->consumersMapKey)->plottedGraphs.clear();

    if(item->checkState() == Qt::Checked)
    {
        auto plotDetails = m_consumersMap[item->consumersMapKey];
        for(auto const consumerKey: plotDetails->graphDataMap.keys())
        {
            // qDebug() << "adding new graph:" << plotDetails->consumerName << consumerKey.powerCycle << plotDetails->graphDataMap[{consumerKey}].pid << consumerKey.metric << "max:" << plotDetails->graphDataMap[{consumerKey}].resourceStats.average;

            if(!m_ecuResetsEnableMap.value({plotDetails->ecu, consumerKey.powerCycle}, false) ||
                m_resourceMonitorEnableMap[consumerKey.metric] != Qt::Checked ||
                !plotDetails->graphDataMap[consumerKey].points)
            {
                continue;
            }
            // qDebug() << "adding new graph:" << plotDetails->consumerName << "done";
            int pid = plotDetails->graphDataMap[{consumerKey}].pid;
            UsageGraph *graph = new UsageGraph(ui->plotter->xAxis, ui->plotter->yAxis, consumerKey.metric);
            graph->setAdaptiveSampling(true);
            graph->setLineStyle(QCPGraph::lsLine);
            graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::yellow, Qt::black, 5));
            if(item->isOverallConsumer)
            {
                graph->setName(QString("%1:%2 %3 %4").arg(plotDetails->ecu, QString::number(consumerKey.powerCycle),
                                                          plotDetails->consumerName,
                                                          m_resourceToStringMap[consumerKey.metric]));
            }
            else
            {
                graph->setName(QString("%1:%2 %3[%4]%5").arg(plotDetails->ecu, QString::number(consumerKey.powerCycle),
                                                             plotDetails->consumerName, QString::number(pid),
                                                             m_resourceToStringMap[consumerKey.metric]));
            }
            graph->setData(plotDetails->graphDataMap[consumerKey].points);
            graph->addToLegend();
            if(m_stackGrpahs)
            {

                graph->setPen(Qt::NoPen);
                graph->setBrush(QBrush(getARandomeColor(70)));
            }
            else
            {
                graph->setPen(getARandomeColor());
            }
            graph->setVisible(true);
            graph->plottableModeltem = item;
            graph->powerCycle = consumerKey.powerCycle;
            ui->plotter->replot();
            plotDetails->plottedGraphs.append(graph);
        }
    }
    else
    {
        if(!m_enabledConsumersMap.contains(item->consumersMapKey))
        {
            m_enabledConsumersMap.remove(item->consumersMapKey);
        }
    }
    m_graphDataQMtx.unlock();
    // if(m_consumersMap.value(item->consumersMapKey)->plottedGraphs.isEmpty())
    // {
    //     changedItem->setCheckState(Qt::Unchecked);
    // }
}

void Form::handleCmbUsageTypeItemChange(QStandardItem *changedItem)
{
    qDebug() << __FUNCTION__ << changedItem->text() << "chenged:" << changedItem->checkState();
    Metric uType = static_cast<Metric>(changedItem->data().value<int>());
    m_resourceMonitorEnableMap[uType] = changedItem->checkState();
    qDebug() << changedItem->text() << "item changed to:" << changedItem->checkState() << "type" << uType;

    for (int row = 0; row < m_graphItemsModel->rowCount(); ++row)
    {
        auto *item = m_graphItemsModel->item(row, 0);
        if (item)
        {
            handleCmbPlottablesItemChange(item);
        }
    }
}

void Form::handleEcuResetComboBoxItemChange(QStandardItem *changedItem)
{
    QListView* view = qobject_cast<QListView *>(ui->cmbPlottables->view());
    if(!changedItem || !view)
    {
        return;
    }
    auto key = changedItem->data().value<QPair<QString, int>>();
    // qDebug() << __FUNCTION__ << changedItem->text() << "chenged:" << changedItem->checkState();
    if(key.first.isEmpty())
    {
        return;
    }
    m_ecuResetsEnableMap[key] = (changedItem->checkState() == Qt::Checked);

    for (int row = 0; row < m_graphItemsModel->rowCount(); ++row)
    {
        auto *item = m_graphItemsModel->item(row, 0);
        if (item)
        {
            handleCmbPlottablesItemChange(item);
        }
    }
}

void Form::on_chkHideLowConsumers_toggled(bool checked)
{
    // QListView* view = qobject_cast<QListView *>(ui->cmbPlottables->view());
    // if(!view)
    // {
    //     return;
    // }

    // for(auto &app : m_entityPlotDetailsMap)
    // {
    //     if(app.plottableModeltem->maxUtilization <= 0.05)
    //     {
    //         view->setRowHidden(app.plottableModeltem->row(), checked);
    //     }
    // }
}

void Form::on_cmbPlottables_customContextMenuRequested(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    QAction actSortAlpha("Sort by Name", this);
    QAction actUnselectAll("Unselect All", this);


    auto cbkSortName = [this]()
    {
        m_graphItemsModel->setSortRole(Qt::DisplayRole);
        m_graphItemsModel->sort(0, Qt::AscendingOrder);
    };

    auto cbkUnselectAll = [this]()
    {
        for(auto &app : m_consumersMap)
        {
                app->plottableModeltem->setCheckState(Qt::Unchecked);
        }
    };

    QList<QPair<QAction*, std::function<void()>>> actionFuncPairs;
    for(auto &ut: m_resourceMonitorEnableMap.keys())
    {
        QPair<QAction*, std::function<void()>> actionFunc;
        actionFunc.second = [this, ut]()
        {
            m_graphItemsModel->setSortRole(Qt::UserRole);
            ModelItem::systemResource = ut;
            m_graphItemsModel->sort(0, Qt::DescendingOrder);
        };

        switch (ut) {
        case Metric::TotalCpu:
            actionFunc.first = new QAction("Sort by Average Total CPU Usage", this);
            break;
        case Metric::SystemCpu:
            actionFunc.first = new QAction("Sort by Average System CPU Usage", this);
            break;
        case Metric::UserCpu:
            actionFunc.first = new QAction("Sort by Average User CPU Usage", this);
            break;
        case Metric::TotalMemory:
            actionFunc.first = new QAction("Sort by Average Total Memory Usage", this);
            break;
        case Metric::RssMemory:
            actionFunc.first = new QAction("Sort by Average RSS Memory Usage", this);
            break;
        case Metric::VirtualMemory:
            // actionFunc.first = new QAction("Sort by Average Virtual Memory Usage", this);
            break;
        default:
            continue;
        }
        actionFuncPairs.append(actionFunc);
    }

    for(auto &pair: actionFuncPairs)
    {
        connect(pair.first, &QAction::triggered, this, pair.second);
        contextMenu.addAction(pair.first);
    }

    connect(&actSortAlpha, &QAction::triggered, this, cbkSortName);
    connect(&actUnselectAll, &QAction::triggered, this, cbkUnselectAll);

    contextMenu.addAction(&actSortAlpha);
    contextMenu.addAction(&actUnselectAll);

    contextMenu.exec(ui->cmbPlottables->mapToGlobal(pos));
    for(auto &pair: actionFuncPairs)
    {
        delete pair.first;
    }
}


void Form::on_chkStackGraphs_toggled(bool state)
{
    m_stackGrpahs = state;
    for (const auto &consumer : qAsConst(m_consumersMap))
    {
        for(auto const graph: qAsConst(consumer->plottedGraphs))
        {
            qDebug() << __func__ << "change graph stack:" << graph->name() << state << graph->brush();
            if(m_stackGrpahs)
            {
                graph->setLineStyle(QCPGraph::lsLine);
                graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::yellow, Qt::black, 5));
                graph->setBrush(QBrush(getARandomeColor(70)));
                graph->setPen(Qt::NoPen);
            }
            else
            {
                graph->setBrush(Qt::NoBrush);
                graph->setPen(getARandomeColor());
            }
        }
    }
}

QColor Form::getARandomeColor(int transparency)
{
    static const std::vector<QColor> sColors
        {
            QColor(255, 0, 0, transparency),     // Bright Red
            QColor(0, 255, 0, transparency),     // Bright Green
            QColor(0, 0, 255, transparency),     // Bright Blue
            QColor(0, 128, 0, transparency),     // Dark Green
            QColor(255, 55, 20, transparency),   // Bright Green
            QColor(255, 165, 0, transparency),   // Orange
            QColor(128, 0, 128, transparency),   // Purple
            QColor(0, 255, 255, transparency),   // Cyan
            QColor(139, 69, 19, transparency),   // Saddle Brown
            QColor(255, 20, 147, transparency),  // Deep Pink
            QColor(255, 140, 0, transparency),   // Dark Orange
            QColor(75, 0, 130, transparency),    // Indigo
            QColor(0, 206, 209, transparency),   // Dark Turquoise
            QColor(34, 139, 34, transparency),   // Forest Green
            QColor(210, 105, 30, transparency),  // Chocolate
            QColor(148, 0, 211, transparency),   // Dark Violet
            QColor(0, 0, 139, transparency),     // Dark Blue
            QColor(255, 69, 0, transparency),    // Red-Orange
            QColor(0, 255, 127, transparency),   // Spring Green
            QColor(112, 128, 144, transparency), // Slate Gray
            QColor(0, 191, 255, transparency),   // Deep Sky Blue
            QColor(220, 20, 60, transparency),   // Crimson
            QColor(0, 128, 128, transparency),   // Teal
            QColor(173, 216, 230, transparency), // Light Blue
            QColor(144, 238, 144, transparency)  // Light Green
        };
    static const int sColorsSize = sColors.size();
    static std::atomic<int> sColorsCounter(QRandomGenerator::securelySeeded().bounded(sColorsSize));
    return sColors[sColorsCounter++ % sColorsSize];
}

PointMetadata Form::getPointMetadata(double key, double value) const
{
    return m_pointsMetadataMap.value({key, value}, {Metric::UNKOWN, 0, 0, 0});
}
