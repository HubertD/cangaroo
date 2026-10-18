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

#include <QDialog>
#include <QMap>
#include <QString>
#include <core/CanDbMessage.h>

namespace Ui { class WaveformDialog; }

struct WaveformConfig {
    enum Type { None = 0, Square = 1, Sine = 2, Ramp = 3 };
    Type type = None;

    // --- Square wave ---
    double sqHighVal  = 1.0;    // signal value when HIGH
    double sqLowVal   = 0.0;    // signal value when LOW
    double sqTon_ms   = 500.0;  // duration at HIGH level (ms)
    double sqToff_ms  = 500.0;  // duration at LOW level (ms)

    // --- Sine wave ---
    double sinAmplitude  = 1.0;     // half peak-to-peak deviation
    double sinOffset     = 0.0;     // DC centre value
    double sinPeriod_ms  = 1000.0;  // full cycle duration (ms)
    double sinPhase_deg  = 0.0;     // phase offset (degrees)

    // --- Ramp (sawtooth) ---
    double rampStartVal = 0.0;    // starting signal value
    double rampEndVal   = 1.0;    // ending signal value
    double rampRise_ms  = 1000.0; // time to travel from start to end (ms)
    double rampHold_ms  = 0.0;    // time to hold at end before reset (0 = instant)
};

class WaveformPreviewWidget;

class WaveformDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WaveformDialog(CanDbMessage *dbMsg,
                            const QMap<QString, WaveformConfig> &existing,
                            QWidget *parent = nullptr);
    ~WaveformDialog();

    QMap<QString, WaveformConfig> waveforms() const { return _waveforms; }

private slots:
    void onSignalSelectionChanged();
    void onTypeChanged(int index);
    void onAnyParamChanged();
    void onApplyToAll();

private:
    Ui::WaveformDialog *ui;
    CanDbMessage *_dbMsg;
    QMap<QString, WaveformConfig> _waveforms;
    WaveformPreviewWidget *_preview;
    bool _loading = false;

    void loadSignalConfig(const QString &signalName);
    void saveCurrentSignalConfig();
    QString currentSignalName() const;
    void updatePreview();
};
