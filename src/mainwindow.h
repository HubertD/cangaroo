/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

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

#include <QMainWindow>
#include <QList>
#include <core/Backend.h>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QWidget;
class QSignalMapper;
class QDomElement;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class ConfigurableWidget;
class SetupDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    QMainWindow *createTraceWindow(QString title=QString());
    QMainWindow *createGraphWindow(QString title=QString());
    void addGraphWidget(QMainWindow *parent=0);
    void addRawTxWidget(QMainWindow *parent=0);
    void addLogWidget(QMainWindow *parent=0);
    void addStatusWidget(QMainWindow *parent=0);

    bool showSetupDialog();
    void showAboutDialog();

    void startMeasurement();
    void stopMeasurement();
    void saveTraceToFile();

    void updateMeasurementActions();

private slots:
    void on_action_WorkspaceNew_triggered();
    void on_action_WorkspaceOpen_triggered();
    void on_action_WorkspaceSave_triggered();
    void on_action_WorkspaceSaveAs_triggered();
    void on_action_TraceClear_triggered();

    void on_actionCan_Status_View_triggered();

private:
    Ui::MainWindow *ui;
    SetupDialog *_setupDlg;

    bool _workspaceModified;
    QString _workspaceFileName;
    QString _baseWindowTitle;

    Backend &backend();

    QMainWindow *createTab(QString title);
    QMainWindow *currentTab();

    void stopAndClearMeasurement();

    void clearWorkspace();
    bool loadWorkspaceTab(QDomElement el);
    bool loadWorkspaceSetup(QDomElement el);
    void loadWorkspaceFromFile(QString filename);
    bool saveWorkspaceToFile(QString filename);

    void newWorkspace();
    void loadWorkspace();
    bool saveWorkspace();
    bool saveWorkspaceAs();

    void setWorkspaceModified(bool modified);
    int askSaveBecauseWorkspaceModified();

};
