#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QAction>

class MeasurementSetup;
class MeasurementNetwork;
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
    explicit SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

    MeasurementSetup *showSetupDialog(MeasurementSetup &setup);

public slots:
    void treeViewSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void treeViewContextMenu(const QPoint& pos);

private slots:
    void edNetworkNameChanged();

    void on_btAddInterface_clicked();
    void on_btRemoveInterface_clicked();

    void on_btAddDatabase_clicked();
    void on_btRemoveDatabase_clicked();
    void updateButtons();

    void executeAddCanDb();
    void executeDeleteCanDb();

    void executeAddInterface();
    void executeDeleteInterface();

private:
    Ui::SetupDialog *ui;

    QAction *_actionDeleteInterface;
    QAction *_actionDeleteCanDb;
    QAction *_actionAddInterface;
    QAction *_actionAddCanDb;

    SetupDialogTreeModel *model;
    MeasurementNetwork *_currentNetwork;
    QModelIndex getSelectedIndex();
    SetupDialogTreeItem *getSelectedItem();

    void showInterfacesPage(SetupDialogTreeItem *item);
    void addCanDb(const QModelIndex &parent);
    void addInterface(const QModelIndex &parent);

};

#endif // SETUPDIALOG_H
