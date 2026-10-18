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

#include "ScatterVisualization.h"
#include <QVBoxLayout>
#include <float.h>
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
#include <QMenu>
#include <QFileDialog>
#include <QTextStream>
#include <QtCharts/QValueAxis>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QChartView>
#include <core/ThemeManager.h>

ScatterVisualization::ScatterVisualization(QWidget *parent, Backend &backend)
    : VisualizationWidget(parent, backend), _windowDuration(0), _autoScroll(true), _isUpdatingRange(false)
{
    _updateTimer = new QTimer(this);
    connect(_updateTimer, &QTimer::timeout, this, &ScatterVisualization::onActivated);
    _updateTimer->start(30); // 30Hz sync
    _chart = new QChart();
    _chart->legend()->setVisible(true);
    _chart->legend()->setAlignment(Qt::AlignBottom);
    _chart->setTitle("Scatter (Distribution View)");

    _chartView = new QChartView(_chart);
    _chartView->setRenderHint(QPainter::Antialiasing);
    _chartView->setRubberBand(QChartView::HorizontalRubberBand);
    _chartView->setMouseTracking(true);
    _chartView->viewport()->setMouseTracking(true);
    _chartView->viewport()->installEventFilter(this);
    _chartView->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_chartView);
    layout->setContentsMargins(0, 0, 0, 0);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [s]");
    _chart->addAxis(axisX, Qt::AlignBottom);
    connect(axisX, &QValueAxis::rangeChanged, this, &ScatterVisualization::updateAxes);
    connect(axisX, &QValueAxis::rangeChanged, this, &ScatterVisualization::onAxisRangeChanged);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    _chart->addAxis(axisY, Qt::AlignLeft);

    // Create cursor line
    _cursorLine = new QGraphicsLineItem(_chart);
    QPen cursorPen(palette().color(QPalette::WindowText), 1, Qt::DashLine);
    _cursorLine->setPen(cursorPen);
    _cursorLine->setZValue(1000);
    _cursorLine->hide();

    // Create Tooltip Box
    _tooltipBox = new QGraphicsRectItem(_chart);
    _tooltipBox->setBrush(QBrush(palette().color(QPalette::ToolTipBase)));
    _tooltipBox->setPen(QPen(palette().color(QPalette::ToolTipText), 1));
    _tooltipBox->setZValue(2000);
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

ScatterVisualization::~ScatterVisualization()
{
}

void ScatterVisualization::addDecodedData(const QMap<CanDbSignal*, DecodedSignalData>& newPoints)
{
    for (auto it = newPoints.begin(); it != newPoints.end(); ++it) {
        CanDbSignal* signal = it.key();
        if (!_signals.contains(signal)) continue;

        const DecodedSignalData &data = it.value();
        
        _signalBuffers[signal].timestamps.append(data.timestamps);
        _signalBuffers[signal].values.append(data.values);
        _signalBusMap[signal] = data.interfaceId;

        if (_signalBuffers[signal].timestamps.size() > MAX_POINTS + 10) {
            _signalBuffers[signal].timestamps.remove(0, 10);
            _signalBuffers[signal].values.remove(0, 10);
            _syncIndices[signal] = qMax(0, _syncIndices.value(signal, 0) - 10);
        }
    }
}

void ScatterVisualization::addMessage(const CanMessage &msg)
{
    if (_startTime < 0) {
        setGlobalStartTime(msg.getFloatTimestamp());
    }

    double t = msg.getFloatTimestamp() - _startTime;
    CanInterfaceId msgIfId = msg.getInterfaceId();

    for (CanDbSignal *signal : _signals) {
        if (signal->isPresentInMessage(msg)) {
            // Network Context Filtering
            if (_signalInterfaces.contains(signal)) {
                if (!_signalInterfaces[signal].contains(msgIfId)) {
                    continue;
                }
            }

            double value = signal->extractPhysicalFromMessage(msg);
            
            // Populate isolated buffer (Raw data only)
            _signalBuffers[signal].timestamps.append(t);
            _signalBuffers[signal].values.append(value);
            _signalBusMap[signal] = msg.getInterfaceId();

            if (_signalBuffers[signal].timestamps.size() > MAX_POINTS + 10) {
                _signalBuffers[signal].timestamps.remove(0, 10);
                _signalBuffers[signal].values.remove(0, 10);
                _syncIndices[signal] = qMax(0, _syncIndices.value(signal, 0) - 10);
            }
        }
    }
}

void ScatterVisualization::onActivated()
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
        QScatterSeries *series = it.value();
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
        if (series->count() > MAX_POINTS) {
            int toRemove = series->count() - MAX_POINTS + 10;
            series->removePoints(0, toRemove);
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
    
    updateAxes(); // Trigger Y auto-fit
}

void ScatterVisualization::setSignalColor(CanDbSignal *signal, const QColor &color)
{
    VisualizationWidget::setSignalColor(signal, color);
    if (_seriesMap.contains(signal)) {
        _seriesMap[signal]->setColor(color);
        _seriesMap[signal]->setBrush(QBrush(color));
    }
    if (_tracers.contains(signal)) {
        _tracers[signal]->setBrush(color);
    }
}

void ScatterVisualization::setActive(bool active)
{
    if (active) {
        if (!_updateTimer->isActive()) _updateTimer->start(30);
    } else {
        _updateTimer->stop();
    }
}

void ScatterVisualization::updateAxes()
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

        // Binary search to find points in the visible range
        auto startIt = std::lower_bound(buffer.timestamps.begin(), buffer.timestamps.end(), minX);
        int startIdx = std::distance(buffer.timestamps.begin(), startIt);

        for (int i = startIdx; i < buffer.timestamps.size(); ++i) {
            double tx = buffer.timestamps[i];
            if (tx > maxX) break; // Optimization
            
            double val = buffer.values[i];
            minY = qMin(minY, val);
            maxY = qMax(maxY, val);
            hasData = true;
        }
    }

    if (hasData) {
        double range = maxY - minY;
        if (range < 0.0001) {
            minY -= 0.5;
            maxY += 0.5;
        } else {
            minY -= range * 0.1;
            maxY += range * 0.1;
        }

        // Stabilize axis by rounding to 4 decimal places to prevent flickering ticks
        minY = std::floor(minY * 10000.0) / 10000.0;
        maxY = std::ceil(maxY * 10000.0) / 10000.0;

        _chart->axes(Qt::Vertical).first()->setRange(minY, maxY);
    }
}

void ScatterVisualization::clear()
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
        _chart->axes(Qt::Horizontal).first()->setRange(0, 5);
    }
    if (!_chart->axes(Qt::Vertical).isEmpty()) {
        _chart->axes(Qt::Vertical).first()->setRange(-10, 10);
    }
    _autoScroll = true;
}

void ScatterVisualization::clearSignals()
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

void ScatterVisualization::wheelEvent(QWheelEvent *event)
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
    event->accept();
}

bool ScatterVisualization::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == _chartView || watched == _chartView->viewport()) && event->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        handleHover(me->pos());
        emit mouseMoved(me);
    }
    return VisualizationWidget::eventFilter(watched, event);
}

void ScatterVisualization::handleHover(QPointF pos)
{
    QPointF scenePos = _chartView->mapToScene(pos.toPoint());
    QPointF chartPos = _chart->mapFromScene(scenePos);

    if (!_chart->plotArea().contains(chartPos)) {
        _cursorLine->hide();
        _tooltipBox->hide();
        for (auto tracer : _tracers.values()) tracer->hide();
        return;
    }

    _cursorLine->setLine(chartPos.x(), _chart->plotArea().top(), chartPos.x(), _chart->plotArea().bottom());
    _cursorLine->show();

    QPointF valPos = _chart->mapToValue(chartPos);
    double targetX = valPos.x();

    uint64_t startUsecs = _backend.getUsecsAtMeasurementStart();
    uint64_t currentUsecs = startUsecs + (uint64_t)(targetX * 1000000.0);
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(currentUsecs / 1000);
    QString timeStr = dt.toString("yyyy-MM-dd HH:mm:ss.zzz t");

    QString tooltipHtml = QString("<div style='font-family: Arial; font-size: 11px; padding: 5px; background: %1; color: %2;'>"
                           "<b>%3</b><br/><br/>")
                           .arg(ThemeManager::instance().currentTheme() == ThemeManager::Dark ? "#333333" : "#ffffff")
                           .arg(ThemeManager::instance().currentTheme() == ThemeManager::Dark ? "#ffffff" : "#000000")
                           .arg(timeStr);

    bool hasAnyTracer = false;

    for (CanDbSignal *signal : _signals) {
        if (!_signalBuffers.contains(signal) || _signalBuffers[signal].timestamps.isEmpty()) {
            _tracers[signal]->hide();
            continue;
        }

        const auto &timestamps = _signalBuffers[signal].timestamps;
        const auto &values = _signalBuffers[signal].values;

        auto it = std::lower_bound(timestamps.begin(), timestamps.end(), targetX);
        int idx = std::distance(timestamps.begin(), it);

        if (idx == 0) {
        } else if (idx >= timestamps.size()) {
            idx = timestamps.size() - 1;
        } else {
            if (qAbs(timestamps[idx] - targetX) > qAbs(timestamps[idx - 1] - targetX)) {
                idx--;
            }
        }

        if (idx >= 0 && idx < timestamps.size() && qAbs(timestamps[idx] - targetX) < 0.5) {
            double actualX = timestamps[idx];
            double actualY = values[idx];
            QPointF trPos = _chart->mapToPosition(QPointF(actualX, actualY));
            
            _tracers[signal]->setPos(trPos);
            _tracers[signal]->show();
            
            tooltipHtml += QString("<span style='color:%1; font-size: 14px;'>●</span> (Bus %2) %3: <b>%4</b> %5<br/>")
                            .arg(_seriesMap[signal]->color().name())
                            .arg(getBusId(signal))
                            .arg(signal->name())
                            .arg(actualY, 0, 'f', 2)
                            .arg(signal->getUnit());
            hasAnyTracer = true;
        } else {
            _tracers[signal]->hide();
        }
    }

    tooltipHtml += "</div>";

    if (hasAnyTracer) {
        _tooltipText->setHtml(tooltipHtml);
        QRectF tr = _tooltipText->boundingRect();
        _tooltipBox->setRect(0, 0, tr.width(), tr.height());

        QPointF tpos = chartPos + QPointF(15, -tr.height() - 15);
        if (tpos.x() + tr.width() > _chart->plotArea().right()) tpos.setX(chartPos.x() - tr.width() - 15);
        if (tpos.y() < _chart->plotArea().top()) tpos.setY(chartPos.y() + 15);

        _tooltipBox->setPos(tpos);
        _tooltipBox->show();
    } else {
        _tooltipBox->hide();
    }
}

void ScatterVisualization::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *actCsv = menu.addAction("Export to CSV");
    QAction *actImg = menu.addAction("Export to Image");
    
    QAction *selected = menu.exec(event->globalPos());
    
    if (selected == actCsv) exportToCsv();
    else if (selected == actImg) exportToImage();
}

void ScatterVisualization::exportToCsv()
{
    QString path = QFileDialog::getSaveFileName(this, "Export Scatter CSV", "", "CSV Files (*.csv)");
    if (path.isEmpty()) return;
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    
    QTextStream out(&file);
    out << "Signal,Time[s],Value\\n";
    
    for (CanDbSignal *signal : _signals) {
        if (!_signalBuffers.contains(signal)) continue;
        const auto &buffer = _signalBuffers[signal];
        for (int i = 0; i < buffer.timestamps.size(); ++i) {
            out << signal->name() << "," << buffer.timestamps[i] << "," << buffer.values[i] << "\\n";
        }
    }
}

void ScatterVisualization::exportToImage()
{
    QString path = QFileDialog::getSaveFileName(this, "Export Scatter Image", "", "PNG Images (*.png);;JPEG Images (*.jpeg)");
    if (path.isEmpty()) return;
    
    QPixmap pixmap = _chartView->grab();
    pixmap.save(path);
}

void ScatterVisualization::zoomIn()
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
}

void ScatterVisualization::zoomOut()
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
}

void ScatterVisualization::resetZoom()
{
    _autoScroll = true;
}

void ScatterVisualization::setWindowDuration(int seconds)
{
    _windowDuration = seconds;
    _autoScroll = true;
}

void ScatterVisualization::addSignal(CanDbSignal *signal, const CanInterfaceIdList &interfaces)
{
    if (_seriesMap.contains(signal)) return;

    VisualizationWidget::addSignal(signal, interfaces);

    QScatterSeries *series = new QScatterSeries();
    series->setUseOpenGL(true);
    QString label = signal->name();
    series->setName(label);
    series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    series->setMarkerSize(7.2);
    
    // Assign a color from a list if chart doesn't provide a distinct one yet
    static const QList<Qt::GlobalColor> palette = { Qt::red, Qt::blue, Qt::green, Qt::magenta, Qt::darkYellow, Qt::cyan };
    QColor color = palette[_seriesMap.size() % palette.size()];
    series->setColor(color);
    
    _chart->addSeries(series);
    
    // Explicitly set color to ensure brush is visible
    series->setBrush(QBrush(color));
    series->setPen(QPen(Qt::NoPen)); // Clean scatter look
    
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

void ScatterVisualization::onAxisRangeChanged(qreal min, qreal max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    if (!_isUpdatingRange) {
        _autoScroll = false;
    }
}

void ScatterVisualization::applyTheme(ThemeManager::Theme theme)
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
    
    _chart->setBackgroundBrush(colors.graphBackground);
}
