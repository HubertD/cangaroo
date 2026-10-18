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

#pragma once

#include "VisualizationWidget.h"
#include <QScrollArea>
#include <QGridLayout>
#include <QWidget>
#include <QPainter>

class GaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GaugeWidget(QWidget *parent = nullptr);
    void setSignalName(const QString &name) { _name = name; update(); }
    void setValue(double value);
    void setRange(double min, double max);
    void setUnit(const QString &unit) { _unit = unit; update(); }
    void setColor(const QColor &color) { _color = color; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString _name;
    QString _unit;
    double _value;
    double _min;
    double _max;
    QColor _color;
};

class GaugeVisualization : public VisualizationWidget
{
    Q_OBJECT
public:
    explicit GaugeVisualization(QWidget *parent, Backend &backend);
    virtual ~GaugeVisualization();

    virtual void addMessage(const CanMessage &msg) override;
    virtual void addDecodedData(const QMap<CanDbSignal*, DecodedSignalData>& newPoints) override;
    virtual void clear() override;
    virtual void onActivated() override;
    virtual void addSignal(CanDbSignal *signal, const CanInterfaceIdList &interfaces = {}) override;
    virtual void clearSignals() override;
    virtual void setSignalColor(CanDbSignal *signal, const QColor &color) override;

    void setColumnCount(int count);

private:
    QScrollArea *_scrollArea;
    QWidget *_container;
    QGridLayout *_containerLayout;
    QMap<CanDbSignal*, GaugeWidget*> _gaugeMap;
    int _columnCount;
};
