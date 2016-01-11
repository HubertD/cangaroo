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

#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QAction>

class Backend;
class MeasurementSetup;
class MeasurementNetwork;
class MeasurementInterface;
class QItemSelection;
class SetupDialogTreeItem;
class SetupDialogTreeModel;

namespace Ui {
class SetupDialog;
}

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(Backend &backend, QWidget *parent = 0);
    ~SetupDialog();

    bool showSetupDialog(MeasurementSetup &setup);

public slots:
    void treeViewSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void treeViewContextMenu(const QPoint& pos);

private slots:
    void edNetworkNameChanged();
    void cbInterfaceBitrateChanged(QString value);

    void on_btAddInterface_clicked();
    void on_btRemoveInterface_clicked();

    void on_btAddDatabase_clicked();
    void on_btRemoveDatabase_clicked();
    void updateButtons();

    void executeAddCanDb();
    void executeDeleteCanDb();

    void executeAddInterface();
    void executeDeleteInterface();

    void on_btAddNetwork_clicked();

    void on_btRemoveNetwork_clicked();

private:
    Ui::SetupDialog *ui;
    Backend *_backend;

    QAction *_actionDeleteInterface;
    QAction *_actionDeleteCanDb;
    QAction *_actionAddInterface;
    QAction *_actionAddCanDb;

    SetupDialogTreeModel *model;
    MeasurementNetwork *_currentNetwork;
    MeasurementInterface *_currentInterface;

    QModelIndex getSelectedIndex();
    SetupDialogTreeItem *getSelectedItem();

    void showInterfacePage(SetupDialogTreeItem *item);
    void addCanDb(const QModelIndex &parent);
    void addInterface(const QModelIndex &parent);

};

#endif // SETUPDIALOG_H
