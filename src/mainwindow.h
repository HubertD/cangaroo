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
#include <QShowEvent>
#include <core/Backend.h>
#include <QPushButton>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QWidget;
class QSignalMapper;
class QDomElement;
class QActionGroup;
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

    void setTheme(const QString &theme);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

public slots:
    QMainWindow *createTraceWindow(QString title=QString());
    QMainWindow *createGraphWindow(QString title=QString());
    void createStandaloneGraphWindow();
    void addGraphWidget(QMainWindow *parent=0);
    QDockWidget *addRawTxWidget(QMainWindow *parent=0);
    QDockWidget *addLogWidget(QMainWindow *parent=0);
    QDockWidget *addStatusWidget(QMainWindow *parent=0);
    QDockWidget *addScriptWidget(QMainWindow *parent = nullptr);
    QDockWidget *addTxGeneratorWidget(QMainWindow *parent=0);
    QDockWidget *addReplayWidget(QMainWindow *parent=0);

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
    void on_actionGenerator_View_triggered();
    void on_actionReport_Issue_triggered();
    void showThemeToolbarMenu();

private:
    QString _currentTheme = QStringLiteral("auto");
    QPushButton *_btnTheme;
    bool _themeResolvedAfterShow = false;
    QActionGroup *_themeActionGroup;
    QAction *_actionThemeAuto;
    QAction *_actionThemeLight;
    QAction *_actionThemeDark;

private:
    Ui::MainWindow *ui;
    SetupDialog *_setupDlg;

    bool _workspaceModified;
    QString _workspaceFileName;
    QString _baseWindowTitle;

    Backend &backend();

    QMainWindow *createTab(QString title);
    QMainWindow *currentTab();

    bool _showSetupDialog_first;

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

    QList<class GraphWindow*> _standaloneGraphWindows;
};
