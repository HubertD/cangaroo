#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QAction>

class MeasurementSetup;
class QItemSelection;
class SetupDialogTreeItem;

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

    void addInterface();
    void addCanDb();
    void deleteCanDb();

private:
    Ui::SetupDialog *ui;

    QAction *_actionDeleteInterface;
    QAction *_actionDeleteCanDb;
    QAction *_actionAddInterface;
    QAction *_actionAddCanDb;

    QModelIndex getSelectedIndex();
    SetupDialogTreeItem *getSelectedItem();
};

#endif // SETUPDIALOG_H
