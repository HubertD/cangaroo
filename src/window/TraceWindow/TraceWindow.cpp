/*

  Copyright (c) 2026 Jayachandran Dharuman

  This file is part of CANgaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "TraceWindow.h"
#include "ui_TraceWindow.h"
#include <window/ConditionalLoggingDialog.h>

#include <QDomDocument>
#include <QSortFilterProxyModel>
#include "LinearTraceViewModel.h"
#include "AggregatedTraceViewModel.h"
#include "UnifiedTraceViewModel.h"
#include "TraceFilterModel.h"
#include <QScrollBar>
#include <core/Backend.h>
#include <core/ThemeManager.h>


TraceWindow::TraceWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::TraceWindow),
    _backend(&backend),
    _mode(mode_unified),
    _timestampMode(timestamp_mode_absolute)
{
    ui->setupUi(this);

    _aggregatedTraceViewModel = new AggregatedTraceViewModel(backend);
    _aggregatedProxyModel = new QSortFilterProxyModel(this);
    _aggregatedProxyModel->setSourceModel(_aggregatedTraceViewModel);
    _aggregatedProxyModel->setDynamicSortFilter(false);

    _aggMonitorFilterModel = new TraceFilterModel(this);
    _aggMonitorFilterModel->setSourceModel(_aggregatedProxyModel);

    // Initialize the 4 rolling categories
    UnifiedTraceViewModel::Category cats[] = { 
        UnifiedTraceViewModel::Cat_All, 
        UnifiedTraceViewModel::Cat_UDS, 
        UnifiedTraceViewModel::Cat_J1939 
    };

    QTreeView* trees[] = { ui->treeAgg, ui->treeUds, ui->treeJ1939 };

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);

    for (int i = 0; i < Cat_Count; ++i) {
        _viewModels[i] = new UnifiedTraceViewModel(backend, cats[i]);
        _filterModels[i] = new TraceFilterModel(this);
        _filterModels[i]->setSourceModel(_viewModels[i]);

        QTreeView* tree = trees[i];
        tree->setModel(_filterModels[i]);
        tree->setFont(font);
        tree->setAlternatingRowColors(true);
        tree->setUniformRowHeights(false);
        tree->setRootIsDecorated(true);

        tree->setColumnWidth(BaseTraceViewModel::column_index, 70);
        tree->setColumnWidth(BaseTraceViewModel::column_timestamp, 100);
        tree->setColumnWidth(BaseTraceViewModel::column_channel, 120);
        tree->setColumnWidth(BaseTraceViewModel::column_direction, 50);
        tree->setColumnWidth(BaseTraceViewModel::column_type, 80);
        tree->setColumnWidth(BaseTraceViewModel::column_canid, 110);
        tree->setColumnWidth(BaseTraceViewModel::column_sender, 150);
        tree->setColumnWidth(BaseTraceViewModel::column_name, 150);
        tree->setColumnWidth(BaseTraceViewModel::column_dlc, 50);
        tree->setColumnWidth(BaseTraceViewModel::column_data, 250);
        tree->setColumnWidth(BaseTraceViewModel::column_comment, 120);

        connect(_filterModels[i], &QAbstractItemModel::rowsInserted, this, &TraceWindow::onRowsInserted);
    }

    // Special handling for the first tab: Can show either Aggregated (Monitor) or Unified (All)
    ui->cbViewMode->addItem(tr("Aggregated"), mode_aggregated);
    ui->cbViewMode->addItem(tr("Rolling Log"), mode_unified);


    ui->cbTimestampMode->addItem(tr("Absolute"), 0);
    ui->cbTimestampMode->addItem(tr("Relative"), 1);
    ui->cbTimestampMode->addItem(tr("Delta"), 2);
    setTimestampMode(timestamp_mode_delta);

    connect(&ThemeManager::instance(), &ThemeManager::traceTextColorChanged, this, &TraceWindow::onTraceTextColorChanged);
    onTraceTextColorChanged(ThemeManager::instance().customTraceTextColor());

    connect(ui->filterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(on_cbFilterChanged()));
    connect(ui->TraceClearpushButton, SIGNAL(released()), this, SLOT(on_cbTraceClearpushButton()));
    connect(ui->cbViewMode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cbViewMode_currentIndexChanged(int)));

    setMode(mode_aggregated);
}

TraceWindow::~TraceWindow()
{
    delete ui;
    delete _aggregatedTraceViewModel;
    for (int i = 0; i < Cat_Count; ++i) {
        delete _viewModels[i];
    }
}

void TraceWindow::setMode(TraceWindow::mode_t mode)
{
    _mode = mode;

    if (mode == mode_aggregated) {
        ui->treeAgg->setModel(_aggMonitorFilterModel);
        ui->treeAgg->setSortingEnabled(true);
        ui->treeAgg->sortByColumn(BaseTraceViewModel::column_canid, Qt::AscendingOrder);
    } else {
        ui->treeAgg->setModel(_filterModels[Cat_Aggregated]);
        ui->treeAgg->setSortingEnabled(false);
        ui->treeAgg->setRootIsDecorated(true);
    }

    for (int i = 0; i < ui->cbViewMode->count(); i++) {
        if (ui->cbViewMode->itemData(i).toInt() == mode) {
            ui->cbViewMode->setCurrentIndex(i);
            break;
        }
    }
    ui->treeAgg->scrollToBottom();
}


void TraceWindow::setTimestampMode(int mode)
{
    timestamp_mode_t new_mode;
    if ( (mode>=0) && (mode<timestamp_modes_count) )
    {
        new_mode = (timestamp_mode_t) mode;
    }
    else
    {
        new_mode = timestamp_mode_absolute;
    }

    _aggregatedTraceViewModel->setTimestampMode(new_mode);
    for (int i = 0; i < Cat_Count; ++i) {
        _viewModels[i]->setTimestampMode(new_mode);
    }

    if (new_mode != _timestampMode)
    {
        _timestampMode = new_mode;
        for (int i=0; i<ui->cbTimestampMode->count(); i++)
        {
            if (ui->cbTimestampMode->itemData(i).toInt() == new_mode)
            {
                ui->cbTimestampMode->setCurrentIndex(i);
            }
        }
        emit(settingsChanged(this));
    }
}

bool TraceWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root))
    {
        return false;
    }

    root.setAttribute("type", "TraceWindow");
    root.setAttribute("mode", _mode == mode_unified ? "unified" : "aggregated");
    root.setAttribute("TimestampMode", _timestampMode);
    root.setAttribute("ActiveTab", ui->tabs->currentIndex());

    QDomElement elAggregated = xml.createElement("AggregatedTraceView");
    elAggregated.setAttribute("SortColumn", _aggregatedProxyModel->sortColumn());
    root.appendChild(elAggregated);

    return true;
}

bool TraceWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el))
    {
        return false;
    }

    QString modeStr = el.attribute("mode", "unified");
    setMode(modeStr == "unified" ? mode_unified : mode_aggregated);
    setTimestampMode(el.attribute("TimestampMode", "0").toInt());
    ui->tabs->setCurrentIndex(el.attribute("ActiveTab", "0").toInt());

    QDomElement elAggregated = el.firstChildElement("AggregatedTraceView");
    int sortColumn = elAggregated.attribute("SortColumn", "-1").toInt();
    ui->treeAgg->sortByColumn(sortColumn, Qt::AscendingOrder);

    return true;
}

void TraceWindow::addMessage(const CanMessage &msg)
{
    _backend->getTrace()->enqueueMessage(msg);
}

void TraceWindow::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;

    TraceFilterModel *filterModel = qobject_cast<TraceFilterModel*>(sender());
    QTreeView* trees[] = { ui->treeAgg, ui->treeUds, ui->treeJ1939 };
    
    // Find which tree corresponds to this filter model and scroll it
    for (int i = 0; i < Cat_Count; ++i) {
        if (_filterModels[i] == filterModel || (i == 0 && _aggMonitorFilterModel == filterModel)) {
            QTreeView* tree = trees[i];
            QScrollBar *vbar = tree->verticalScrollBar();
            bool atBottom = (vbar->value() >= vbar->maximum() - 10); // buffer of 10 pixels
            
            if (atBottom || vbar->maximum() == 0) {
                tree->scrollToBottom();
            }
            break;
        }
    }

    if(_backend->getTrace()->size() > 1000000)
    {
        _backend->clearTrace();
    }
}



void TraceWindow::on_cbTimestampMode_currentIndexChanged(int index)
{
    setTimestampMode((timestamp_mode_t)ui->cbTimestampMode->itemData(index).toInt());
}

void TraceWindow::on_cbFilterChanged()
{
    QString filterText = ui->filterLineEdit->text();
    _aggMonitorFilterModel->setFilterText(filterText);
    _aggMonitorFilterModel->invalidate();

    for (int i = 0; i < Cat_Count; ++i) {
        _filterModels[i]->setFilterText(filterText);
        _filterModels[i]->invalidate();
    }
}

void TraceWindow::on_cbTraceClearpushButton()
{
    _backend->clearTrace();
    // clearTrace() triggers beforeClear/afterClear signals which the models are connected to.
    // However, since we have multiple models, we should ensure they all reset correctly.
    _backend->clearLog();
}

void TraceWindow::on_cbViewMode_currentIndexChanged(int index)
{
    setMode((mode_t)ui->cbViewMode->itemData(index).toInt());
}

void TraceWindow::onTraceTextColorChanged(QColor color)
{
    QString qss = color.isValid() ? QString("QTreeView { color: %1; }").arg(color.name()) : QString("");
    ui->treeAgg->setStyleSheet(qss);
    ui->treeUds->setStyleSheet(qss);
    ui->treeJ1939->setStyleSheet(qss);
}

