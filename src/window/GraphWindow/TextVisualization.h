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
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMap>

class TextVisualization : public VisualizationWidget
{
    Q_OBJECT
public:
    explicit TextVisualization(QWidget *parent, Backend &backend);
    virtual ~TextVisualization();

    virtual void addMessage(const CanMessage &msg) override;
    virtual void addDecodedData(const QMap<CanDbSignal*, DecodedSignalData>& newPoints) override;
    virtual void clear() override;
    virtual void onActivated() override;
    virtual void addSignal(CanDbSignal *signal, const CanInterfaceIdList &interfaces = {}) override;
    virtual void clearSignals() override;
    virtual void setSignalColor(CanDbSignal *signal, const QColor &color) override;
    virtual void applyTheme(ThemeManager::Theme theme) override;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateUi();

private:
    void updateFontScaling();
    void createSignalCard(CanDbSignal *signal);

    struct SignalData {
        double value;
        bool updated;
        QLabel *valueLabel;
        QWidget *card;
    };

    QScrollArea *_scrollArea;
    QWidget *_container;
    QVBoxLayout *_containerLayout;
    QTimer *_updateTimer;
    QMap<CanDbSignal*, SignalData> _signalDataMap;
};
