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
#include <QTreeWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <core/ThemeManager.h>
#include <core/Backend.h>
#include <core/CanDbSignal.h>

class SignalSelectorDialog : public QDialog {
    Q_OBJECT
public:
    explicit SignalSelectorDialog(QWidget *parent, Backend &backend);
    struct SelectedSignal {
        CanDbSignal *signal;
        CanInterfaceIdList interfaces;
    };

    QList<SelectedSignal> getSelectedSignalsWithContext() const;
    void setSelectedSignals(const QList<CanDbSignal*> &sigList);

private slots:
    void onSearchTextChanged(const QString &text);
    void onShowSelectedOnlyToggled(bool checked);
    void onItemChanged(QTreeWidgetItem *item, int column);
    void applyTheme(ThemeManager::Theme theme);

private:
    Backend &_backend;
    QLineEdit *_searchEdit;
    QTreeWidget *_tree;
    QCheckBox *_showSelectedOnly;

    void populateTree();
    void filterTree(const QString &searchText, bool showSelectedOnly);
    bool shouldShowItem(QTreeWidgetItem *item, const QString &searchText, bool showSelectedOnly);
};
