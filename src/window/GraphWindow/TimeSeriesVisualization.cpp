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

#include "TimeSeriesVisualization.h"
#include <QVBoxLayout>
#include <float.h>
#include <QToolTip>
#include <QMouseEvent>
#include <QEvent>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QPen>
#include <QBrush>
#include <QDateTime>
#include <QTimer>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <core/ThemeManager.h>

TimeSeriesVisualization::TimeSeriesVisualization(QWidget *parent, Backend &backend)
    : VisualizationWidget(parent, backend), _windowDuration(0), _autoScroll(true), _isUpdatingRange(false)
{
    _updateTimer = new QTimer(this);
    connect(_updateTimer, &QTimer::timeout, this, &TimeSeriesVisualization::onActivated);
    _updateTimer->start(30);
    _chart = new QChart();
    _chart->legend()->setVisible(true);
    _chart->legend()->setAlignment(Qt::AlignBottom);
    _chart->setTitle("Time Series Graph");

    _chartView = new QChartView(_chart);
    _chartView->setRenderHint(QPainter::Antialiasing);
    _chartView->setRubberBand(QChartView::HorizontalRubberBand);
    _chartView->setMouseTracking(true);
    _chartView->viewport()->setMouseTracking(true); // Ensure viewport tracks mouse
    _chartView->viewport()->installEventFilter(this); // Catch events on viewport
    _chartView->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_chartView);
    layout->setContentsMargins(0, 0, 0, 0);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [s]");
    _chart->addAxis(axisX, Qt::AlignBottom);
    connect(axisX, &QValueAxis::rangeChanged, this, &TimeSeriesVisualization::updateYAxisRange);
    connect(axisX, &QValueAxis::rangeChanged, this, &TimeSeriesVisualization::onAxisRangeChanged);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    _chart->addAxis(axisY, Qt::AlignLeft);
    
    // Create cursor line
    _cursorLine = new QGraphicsLineItem(_chart);
    QPen cursorPen(palette().color(QPalette::WindowText), 1, Qt::DashLine);
    _cursorLine->setPen(cursorPen);
    _cursorLine->setZValue(1000); // High Z-value to be on top
    _cursorLine->hide();

    // Create Tooltip Box
    _tooltipBox = new QGraphicsRectItem(_chart);
    _tooltipBox->setBrush(QBrush(palette().color(QPalette::ToolTipBase)));
    _tooltipBox->setPen(QPen(palette().color(QPalette::ToolTipText), 1));
    _tooltipBox->setZValue(2000); // Very high Z-value
    _tooltipBox->hide();

    _tooltipText = new QGraphicsTextItem(_tooltipBox);
    _tooltipText->setDefaultTextColor(palette().color(QPalette::ToolTipText));
    _tooltipText->setTextInteractionFlags(Qt::NoTextInteraction);
    _tooltipText->setZValue(2001);

    // Add shadow effect to tooltip box
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setOffset(3, 3);
    shadow->setColor(QColor(0, 0, 0, 80));
    _tooltipBox->setGraphicsEffect(shadow);

    setMouseTracking(true);
    applyTheme(ThemeManager::instance().currentTheme());
}

TimeSeriesVisualization::~TimeSeriesVisualization()
{
}

void TimeSeriesVisualization::addDecodedData(const QMap<CanDbSignal*, DecodedSignalData>& newPoints)
{
    for (auto it = newPoints.begin(); it != newPoints.end(); ++it) {
        CanDbSignal* signal = it.key();
        if (!_signals.contains(signal)) continue;

        const DecodedSignalData &data = it.value();
        
        _signalBuffers[signal].timestamps.append(data.timestamps);
        _signalBuffers[signal].values.append(data.values);
        _signalBusMap[signal] = data.interfaceId;

        if (_signalBuffers[signal].timestamps.size() > 210000) {
            _signalBuffers[signal].timestamps.remove(0, 10000);
            _signalBuffers[signal].values.remove(0, 10000);
            _syncIndices[signal] = qMax(0, _syncIndices.value(signal, 0) - 10000);
        }
    }
}

void TimeSeriesVisualization::addMessage(const CanMessage &msg)
{
    // Keeping for backwards compatibility if needed directly
    double timestamp = msg.getFloatTimestamp();

    if (_startTime < 0) {
        setGlobalStartTime(timestamp);
    }

    double t = timestamp - _startTime;
    CanInterfaceId msgIfId = msg.getInterfaceId();

    for (CanDbSignal *signal : _signals) {
        if (signal->isPresentInMessage(msg)) {
            // Network Context Filtering:
            if (_signalInterfaces.contains(signal)) {
                if (!_signalInterfaces[signal].contains(msgIfId)) {
                    continue;
                }
            }

            double value = signal->extractPhysicalFromMessage(msg);
            
            _signalBuffers[signal].timestamps.append(t);
            _signalBuffers[signal].values.append(value);
            _signalBusMap[signal] = msg.getInterfaceId();

            if (_signalBuffers[signal].timestamps.size() > 210000) {
                _signalBuffers[signal].timestamps.remove(0, 10000);
                _signalBuffers[signal].values.remove(0, 10000);
                _syncIndices[signal] = qMax(0, _syncIndices.value(signal, 0) - 10000);
            }
        }
    }
}

void TimeSeriesVisualization::onActivated()
{
    // Find latest timestamp across all signal buffers for scrolling
    double latestT = 0;
    for (const auto &buffer : _signalBuffers.values()) {
        if (!buffer.timestamps.isEmpty()) {
            latestT = qMax(latestT, buffer.timestamps.last());
        }
    }

    // Synchronize series with buffers (Batch Update)
    for (auto it = _seriesMap.begin(); it != _seriesMap.end(); ++it) {
        CanDbSignal *sig = it.key();
        QLineSeries *series = it.value();
        const auto &buffer = _signalBuffers[sig];
        int syncIdx = _syncIndices.value(sig, 0);

        if (syncIdx < buffer.timestamps.size()) {
            QList<QPointF> points;
            points.reserve(buffer.timestamps.size() - syncIdx);
            for (int i = syncIdx; i < buffer.timestamps.size(); ++i) {
                points.append(QPointF(buffer.timestamps[i], buffer.values[i]));
            }
            series->append(points);
            _syncIndices[sig] = buffer.timestamps.size();
        }

        // Keep series size in sync with buffer block pruning
        if (series->count() > 200000) {
            series->removePoints(0, series->count() - 190000);
        }
    }
    
    if (!_autoScroll || _chart->axes(Qt::Horizontal).isEmpty()) return;
    
    double t = latestT;

    _isUpdatingRange = true;
    QAbstractAxis *axisX = _chart->axes(Qt::Horizontal).first();
    if (_windowDuration > 0) {
        double windowSize = (double)_windowDuration;
        if (t > windowSize) {
            axisX->setRange(t - windowSize, t);
        } else {
            axisX->setRange(0, windowSize);
        }
    } else {
        // "All" Mode: Show from 0 to current timestamp
        axisX->setRange(0, qMax(10.0, t));
    }
    _isUpdatingRange = false;

    updateYAxisRange();
}

void TimeSeriesVisualization::setSignalColor(CanDbSignal *signal, const QColor &color)
{
    VisualizationWidget::setSignalColor(signal, color);
    if (_seriesMap.contains(signal)) {
        _seriesMap[signal]->setColor(color);
    }
    if (_tracers.contains(signal)) {
        _tracers[signal]->setBrush(color);
    }
}

void TimeSeriesVisualization::updateYAxisRange()
{
    if (_chart->axes(Qt::Vertical).isEmpty() || _seriesMap.isEmpty()) return;

    double minY = DBL_MAX;
    double maxY = -DBL_MAX;
    bool hasData = false;

    QValueAxis *axisX = qobject_cast<QValueAxis*>(_chart->axes(Qt::Horizontal).first());
    double minX = axisX->min();
    double maxX = axisX->max();

    for (auto it = _signalBuffers.begin(); it != _signalBuffers.end(); ++it) {
        const auto &buffer = it.value();
        if (buffer.timestamps.isEmpty()) continue;

        // Binary search to find the first point in the visible window
        auto startIt = std::lower_bound(buffer.timestamps.begin(), buffer.timestamps.end(), minX);
        int startIdx = std::distance(buffer.timestamps.begin(), startIt);

        for (int i = startIdx; i < buffer.timestamps.size(); ++i) {
            double tx = buffer.timestamps[i];
            if (tx > maxX) break; // Optimization: Stop once we exceed visible range
            
            double val = buffer.values[i];
            minY = qMin(minY, val);
            maxY = qMax(maxY, val);
            hasData = true;
        }
    }

    if (hasData) {
        double range = maxY - minY;
        if (range < 0.0001) { // Very small range
            minY -= 0.5;
            maxY += 0.5;
        } else {
            // Apply 10% padding
            minY -= range * 0.1;
            maxY += range * 0.1;
        }
        
        // Stabilize axis by rounding to 4 decimal places to prevent flickering ticks
        minY = std::floor(minY * 10000.0) / 10000.0;
        maxY = std::ceil(maxY * 10000.0) / 10000.0;

        _chart->axes(Qt::Vertical).first()->setRange(minY, maxY);
    }
}

void TimeSeriesVisualization::clear()
{
    for (auto series : _seriesMap.values()) {
        series->clear();
    }
    for (auto it = _signalBuffers.begin(); it != _signalBuffers.end(); ++it) {
        it.value().timestamps.clear();
        it.value().values.clear();
    }
    for (auto it = _syncIndices.begin(); it != _syncIndices.end(); ++it) {
        *it = 0;
    }
    _startTime = -1;

    if (!_chart->axes(Qt::Horizontal).isEmpty()) {
        _chart->axes(Qt::Horizontal).first()->setRange(0, 5); // Fallback range
    }
    if (!_chart->axes(Qt::Vertical).isEmpty()) {
        _chart->axes(Qt::Vertical).first()->setRange(-10, 10);
    }
    _autoScroll = true;
}

void TimeSeriesVisualization::clearSignals()
{
    for (auto series : _seriesMap.values()) {
        _chart->removeSeries(series);
        delete series;
    }
    for (auto tracer : _tracers.values()) {
        delete tracer;
    }
    _tracers.clear();
    _seriesMap.clear();
    _signals.clear();
    _signalBusMap.clear();
    _signalBuffers.clear();
    _syncIndices.clear();
    _startTime = -1;

    if (!_chart->axes(Qt::Horizontal).isEmpty()) {
        _chart->axes(Qt::Horizontal).first()->setRange(0, 5);
    }
    if (!_chart->axes(Qt::Vertical).isEmpty()) {
        _chart->axes(Qt::Vertical).first()->setRange(-10, 10);
    }
    _autoScroll = true;
}

void TimeSeriesVisualization::wheelEvent(QWheelEvent *event)
{
    _autoScroll = false;
    
    QValueAxis *axisX = qobject_cast<QValueAxis*>(_chart->axes(Qt::Horizontal).first());
    if (axisX) {
        double min = axisX->min();
        double max = axisX->max();
        double center = min + (max - min) * 0.5;
        double factor = (event->angleDelta().y() > 0) ? 0.8 : 1.2;
        
        double newRange = (max - min) * factor;
        _isUpdatingRange = true;
        axisX->setRange(center - newRange * 0.5, center + newRange * 0.5);
        _isUpdatingRange = false;
    }
    
    updateYAxisRange();
    event->accept();
}

bool TimeSeriesVisualization::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == _chartView || watched == _chartView->viewport()) && event->type() == QEvent::MouseMove) {
        emit mouseMoved(static_cast<QMouseEvent*>(event));
    }
    return VisualizationWidget::eventFilter(watched, event);
}

void TimeSeriesVisualization::zoomIn()
{
    _autoScroll = false;
    QValueAxis *axisX = qobject_cast<QValueAxis*>(_chart->axes(Qt::Horizontal).first());
    if (axisX) {
        double min = axisX->min();
        double max = axisX->max();
        double newRange = (max - min) * 0.8;
        double center = min + (max - min) * 0.5;
        _isUpdatingRange = true;
        axisX->setRange(center - newRange * 0.5, center + newRange * 0.5);
        _isUpdatingRange = false;
    }
    updateYAxisRange();
}

void TimeSeriesVisualization::zoomOut()
{
    _autoScroll = false;
    QValueAxis *axisX = qobject_cast<QValueAxis*>(_chart->axes(Qt::Horizontal).first());
    if (axisX) {
        double min = axisX->min();
        double max = axisX->max();
        double newRange = (max - min) * 1.2;
        double center = min + (max - min) * 0.5;
        _isUpdatingRange = true;
        axisX->setRange(center - newRange * 0.5, center + newRange * 0.5);
        _isUpdatingRange = false;
    }
    updateYAxisRange();
}

void TimeSeriesVisualization::resetZoom()
{
    _autoScroll = true;
}

void TimeSeriesVisualization::setWindowDuration(int seconds)
{
    _windowDuration = seconds;
    _autoScroll = true;
}

void TimeSeriesVisualization::addSignal(CanDbSignal *signal, const CanInterfaceIdList &interfaces)
{
    if (_seriesMap.contains(signal)) return;

    VisualizationWidget::addSignal(signal, interfaces);

    QLineSeries *series = new QLineSeries();
    series->setUseOpenGL(true); // QtCharts GPU Optimization
    series->setName(signal->name());
    _chart->addSeries(series);
    
    if (!_chart->axes(Qt::Horizontal).isEmpty()) {
        series->attachAxis(_chart->axes(Qt::Horizontal).first());
    }
    if (!_chart->axes(Qt::Vertical).isEmpty()) {
        series->attachAxis(_chart->axes(Qt::Vertical).first());
    }

    _seriesMap[signal] = series;

    QGraphicsEllipseItem *tracer = new QGraphicsEllipseItem(-4, -4, 8, 8, _chart);
    tracer->setBrush(series->color());
    tracer->setPen(QPen(Qt::white, 1));
    tracer->setZValue(1500);
    tracer->hide();
    _tracers[signal] = tracer;

    // Fix visibility bug: Force auto-scroll so the axis immediately tracks to the new live data point
    _autoScroll = true;
}

void TimeSeriesVisualization::onAxisRangeChanged(qreal min, qreal max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    if (!_isUpdatingRange) {
        _autoScroll = false;
    }
}

void TimeSeriesVisualization::setActive(bool active)
{
    if (active) {
        if (!_updateTimer->isActive()) _updateTimer->start(30);
    } else {
        _updateTimer->stop();
    }
}

void TimeSeriesVisualization::applyTheme(ThemeManager::Theme theme)
{
    const ThemeColors& colors = ThemeManager::instance().colors();
    
    _chart->setBackgroundBrush(colors.graphBackground);
    _chart->setTitleBrush(colors.windowText);
    _chart->legend()->setLabelColor(colors.windowText);

    for (auto axis : _chart->axes()) {
        axis->setLabelsColor(colors.graphAxisText);
        axis->setTitleBrush(colors.graphAxisText);
        if (auto vAxis = qobject_cast<QValueAxis*>(axis)) {
            vAxis->setGridLineColor(colors.graphGrid);
        }
    }

    _cursorLine->setPen(QPen(colors.graphCursor, 1, Qt::DashLine));
    _tooltipBox->setBrush(QBrush(colors.toolTipBase));
    _tooltipBox->setPen(QPen(colors.toolTipText, 1));
    _tooltipText->setDefaultTextColor(colors.toolTipText);

    if (theme == ThemeManager::Dark) {
        _chart->setTheme(QChart::ChartThemeDark);
    } else {
        _chart->setTheme(QChart::ChartThemeLight);
    }
    
    // Explicitly override background again as ChartTheme might change it
    _chart->setBackgroundBrush(colors.graphBackground);
}
