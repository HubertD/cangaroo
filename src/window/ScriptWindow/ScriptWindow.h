/*
  Copyright (c) 2026 Schildkroet

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

#include "core/ConfigurableWidget.h"
#include "core/Backend.h"
#include <QDateTime>

class PythonEngine;
class QPlainTextEdit;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QSplitter;

class ScriptWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit ScriptWindow(QWidget *parent, Backend &backend);
    ~ScriptWindow() override;

    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root) override;
    bool loadXML(Backend &backend, QDomElement &el) override;

protected:


private slots:
    void onRunClicked();
    void onStopClicked();
    void onClearClicked();
    void onLoadClicked();
    void onSaveClicked();
    void onScriptOutput(const QString &text);
    void onScriptError(const QString &text);
    void onScriptStarted();
    void onScriptFinished();
    void onMeasurementStarted();
    void onMeasurementStopped();

private:
    Backend *_backend;
    PythonEngine *_engine;

    QSplitter *_splitter;
    QPlainTextEdit *_editor;
    QPlainTextEdit *_console;
    QPushButton *_btnRun;
    QPushButton *_btnStop;
    QPushButton *_btnClear;
    QPushButton *_btnLoad;
    QPushButton *_btnSave;
    QCheckBox *_chkAutoRun;
    QLineEdit *_fileLabel;
    QString _scriptFilePath;
    QDateTime _lastLoadTime;

    void loadScriptFile(const QString &filename);
    void reloadIfModified();
};
