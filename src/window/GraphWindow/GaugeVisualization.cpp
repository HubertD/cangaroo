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

#include "GaugeVisualization.h"
#include <QVBoxLayout>
#include <QtMath>

GaugeWidget::GaugeWidget(QWidget *parent)
    : QWidget(parent), _value(0), _min(0), _max(100), _color(0, 123, 255)
{
    setMinimumSize(220, 220);
}

void GaugeWidget::setValue(double value)
{
    _value = qBound(_min, value, _max);
    update();
}

void GaugeWidget::setRange(double min, double max)
{
    if (qAbs(max - min) < 0.001) {
        _min = min;
        _max = min + 100;
    } else {
        _min = min;
        _max = max;
    }
    update();
}

void GaugeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    // Draw background ring (Ghost Bar)
    QColor ghostColor = palette().color(QPalette::WindowText);
    ghostColor.setAlpha(30);
    painter.setPen(QPen(ghostColor, 18, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(-85, -85, 170, 170, -30 * 16, 240 * 16);

    // Active Radial Bar (The colored arc)
    double range = _max - _min;
    double ratio = (_value - _min) / range;
    int span = (int)(ratio * 240 * 16);
    
    painter.setPen(QPen(_color, 18, Qt::SolidLine, Qt::RoundCap));
    // Draw from left (210 degrees) clockwise
    painter.drawArc(-85, -85, 170, 170, 210 * 16 - span, span);

    // Text: Value (Primary Readout, Centered)
    painter.setPen(palette().color(QPalette::WindowText));
    QFont font = painter.font();
    font.setPixelSize(40);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(-75, -35, 150, 45), Qt::AlignCenter, QString::number(_value, 'f', 1));

    // Text: Unit (Centered below value)
    font.setPixelSize(16);
    font.setBold(false);
    painter.setFont(font);
    QColor unitColor = palette().color(QPalette::WindowText);
    unitColor.setAlpha(160);
    painter.setPen(unitColor);
    painter.drawText(QRect(-70, 15, 140, 20), Qt::AlignCenter, _unit);

    // Text: Name (Bottom Center)
    font.setPixelSize(13);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawText(QRect(-100, 75, 200, 25), Qt::AlignCenter, _name);
}

GaugeVisualization::GaugeVisualization(QWidget *parent, Backend &backend)
    : VisualizationWidget(parent, backend), _columnCount(2)
{
    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setFrameShape(QFrame::NoFrame);

    _container = new QWidget();
    _containerLayout = new QGridLayout(_container);
    _containerLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _containerLayout->setSpacing(20);

    _scrollArea->setWidget(_container);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_scrollArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

GaugeVisualization::~GaugeVisualization()
{
}

void GaugeVisualization::addDecodedData(const QMap<CanDbSignal*, DecodedSignalData>& newPoints)
{
    for (auto it = newPoints.begin(); it != newPoints.end(); ++it) {
        CanDbSignal* signal = it.key();
        if (!_signals.contains(signal)) continue;

        const DecodedSignalData &data = it.value();
        if (data.timestamps.isEmpty()) continue;
        
        _signalBuffers[signal].timestamps.append(data.timestamps);
        _signalBuffers[signal].values.append(data.values);

        if (_gaugeMap.contains(signal)) {
            _gaugeMap[signal]->setValue(data.values.last());
        }

        if (_signalBuffers[signal].values.size() > 100) {
            int removeCount = _signalBuffers[signal].values.size() - 80;
            _signalBuffers[signal].timestamps.remove(0, removeCount);
            _signalBuffers[signal].values.remove(0, removeCount);
        }
    }
}

void GaugeVisualization::addMessage(const CanMessage &msg)
{
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
            
            // Populate isolated buffer
            _signalBuffers[signal].timestamps.append(msg.getFloatTimestamp());
            _signalBuffers[signal].values.append(value);

            if (_gaugeMap.contains(signal)) {
                _gaugeMap[signal]->setValue(value);
            }
            
            if (_signalBuffers[signal].values.size() > 100) {
                _signalBuffers[signal].timestamps.remove(0);
                _signalBuffers[signal].values.remove(0);
            }
        }
    }
}

void GaugeVisualization::clear()
{
    for (auto gauge : _gaugeMap.values()) {
        gauge->setValue(0);
    }
    _signalBuffers.clear();
}

void GaugeVisualization::onActivated()
{
    VisualizationWidget::onActivated();
    // In a real scenario, we might want to pull last values from Backend buffer
    // For now, we rely on the fact that values are persistent in GaugeWidgets if not cleared
}

void GaugeVisualization::clearSignals()
{
    for (auto gauge : _gaugeMap.values()) {
        _containerLayout->removeWidget(gauge);
        delete gauge;
    }
    _gaugeMap.clear();
    _signals.clear();
}

void GaugeVisualization::addSignal(CanDbSignal *signal, const CanInterfaceIdList &interfaces)
{
    if (_gaugeMap.contains(signal)) return;

    VisualizationWidget::addSignal(signal, interfaces);

    GaugeWidget *gauge = new GaugeWidget(_container);
    gauge->setSignalName(signal->name());
    gauge->setUnit(signal->getUnit());
    
    // Scale based on DBC if available
    double min = signal->getMinimumValue();
    double max = signal->getMaximumValue();
    if (qAbs(max - min) < 0.01) {
        gauge->setRange(0, 100); 
    } else {
        gauge->setRange(min, max);
    }

    _gaugeMap[signal] = gauge;

    int count = _gaugeMap.size() - 1;
    int row = count / _columnCount;
    int col = count % _columnCount;
    _containerLayout->addWidget(gauge, row, col);
    
    // Stretch columns to fill space
    for (int i = 0; i < _columnCount; ++i) {
        _containerLayout->setColumnStretch(i, 1);
    }
}

void GaugeVisualization::setSignalColor(CanDbSignal *signal, const QColor &color)
{
    VisualizationWidget::setSignalColor(signal, color);
    if (_gaugeMap.contains(signal)) {
        _gaugeMap[signal]->setColor(color);
    }
}

void GaugeVisualization::setColumnCount(int count)
{
    if (count < 1 || count > 4) return;
    _columnCount = count;
    
    // Relayout
    QList<CanDbSignal*> sigs = _gaugeMap.keys();
    for (auto signal : sigs) {
        _containerLayout->removeWidget(_gaugeMap[signal]);
    }
    
    for (int i = 0; i < sigs.size(); ++i) {
        int row = i / _columnCount;
        int col = i % _columnCount;
        _containerLayout->addWidget(_gaugeMap[sigs[i]], row, col);
    }
    
    // Update stretches
    for (int i = 0; i < 4; ++i) {
        _containerLayout->setColumnStretch(i, (i < _columnCount) ? 1 : 0);
    }
}
