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

#include "SignalSelectorDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTreeWidgetItemIterator>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/CanDbMessage.h>

SignalSelectorDialog::SignalSelectorDialog(QWidget *parent, Backend &backend)
    : QDialog(parent), _backend(backend)
{
    setWindowTitle(tr("Select Data"));
    setMinimumSize(600, 500);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Search and Filters
    QHBoxLayout *filterLayout = new QHBoxLayout();
    _searchEdit = new QLineEdit(this);
    _searchEdit->setPlaceholderText(tr("Search signals or messages..."));
    filterLayout->addWidget(_searchEdit);

    _showSelectedOnly = new QCheckBox(tr("Show selection only"), this);
    filterLayout->addWidget(_showSelectedOnly);
    
    layout->addLayout(filterLayout);

    // Tree
    _tree = new QTreeWidget(this);
    _tree->setHeaderLabels({tr("Name"), tr("Details"), tr("Comment")});
    _tree->setColumnWidth(0, 250);
    layout->addWidget(_tree);

    connect(_tree, &QTreeWidget::itemChanged, this, &SignalSelectorDialog::onItemChanged);

    connect(this, &SignalSelectorDialog::finished, [this]() {
        disconnect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SignalSelectorDialog::applyTheme);
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SignalSelectorDialog::applyTheme);
    applyTheme(ThemeManager::instance().currentTheme());

    populateTree();

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(_searchEdit, &QLineEdit::textChanged, this, &SignalSelectorDialog::onSearchTextChanged);
    connect(_showSelectedOnly, &QCheckBox::toggled, this, &SignalSelectorDialog::onShowSelectedOnlyToggled);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SignalSelectorDialog::populateTree()
{
    MeasurementSetup &setup = _backend.getSetup();
    
    QTreeWidgetItem *networksItem = new QTreeWidgetItem(_tree);
    networksItem->setText(0, tr("CAN Networks"));
    networksItem->setExpanded(true);

    for (MeasurementNetwork *network : setup.getNetworks()) {
        QTreeWidgetItem *netItem = new QTreeWidgetItem(networksItem);
        netItem->setText(0, network->name());
        
        // Store interfaces for this network
        CanInterfaceIdList interfaces = network->getReferencedCanInterfaces();
        QVariant interfaceData = QVariant::fromValue(interfaces);

        for (pCanDb db : network->_canDbs) {
            for (CanDbMessage *msg : db->getMessageList().values()) {
                QTreeWidgetItem *msgItem = new QTreeWidgetItem(netItem);
                msgItem->setText(0, QString("%1 (0x%2)").arg(msg->getName()).arg(msg->getRaw_id(), 0, 16));
                msgItem->setCheckState(0, Qt::Unchecked); // Select All checkbox
                
                for (CanDbSignal *sig : msg->getSignals()) {
                    QTreeWidgetItem *sigItem = new QTreeWidgetItem(msgItem);
                    sigItem->setText(0, sig->name());
                    
                    QString details = QString("%1 | %2..%3 %4")
                        .arg(sig->isUnsigned() ? tr("Unsigned") : tr("Signed"))
                        .arg(sig->getMinimumValue())
                        .arg(sig->getMaximumValue())
                        .arg(sig->getUnit());
                    
                    sigItem->setText(1, details);
                    sigItem->setText(2, sig->comment());
                    sigItem->setCheckState(0, Qt::Unchecked);
                    sigItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)sig));
                    sigItem->setData(0, Qt::UserRole + 1, interfaceData);

                    // Add colored legend icon (deterministic color based on name)
                    QPixmap pix(12, 12);
                    uint h = qHash(sig->name());
                    QColor c = QColor::fromHsl(h % 360, 180, 150);
                    pix.fill(c);
                    sigItem->setIcon(0, QIcon(pix));
                }
            }
        }
    }
}

QList<SignalSelectorDialog::SelectedSignal> SignalSelectorDialog::getSelectedSignalsWithContext() const
{
    QList<SelectedSignal> selected;
    QTreeWidgetItemIterator it(_tree);
    while (*it) {
        if ((*it)->checkState(0) == Qt::Checked) {
            void* sigPtr = (*it)->data(0, Qt::UserRole).value<void*>();
            if (sigPtr) {
                SelectedSignal s;
                s.signal = (CanDbSignal*)sigPtr;
                s.interfaces = (*it)->data(0, Qt::UserRole + 1).value<CanInterfaceIdList>();
                selected.append(s);
            }
        }
        ++it;
    }
    return selected;
}

void SignalSelectorDialog::setSelectedSignals(const QList<CanDbSignal*> &sigList)
{
    QTreeWidgetItemIterator it(_tree);
    while (*it) {
        void* sigPtr = (*it)->data(0, Qt::UserRole).value<void*>();
        if (sigPtr && sigList.contains((CanDbSignal*)sigPtr)) {
            (*it)->setCheckState(0, Qt::Checked);
            
            // Expand parents
            QTreeWidgetItem *p = (*it)->parent();
            while (p) {
                p->setExpanded(true);
                p = p->parent();
            }
        }
        ++it;
    }
}

void SignalSelectorDialog::onSearchTextChanged(const QString &text)
{
    filterTree(text, _showSelectedOnly->isChecked());
}

void SignalSelectorDialog::onShowSelectedOnlyToggled(bool checked)
{
    filterTree(_searchEdit->text(), checked);
}

void SignalSelectorDialog::filterTree(const QString &searchText, bool showSelectedOnly)
{
    QTreeWidgetItemIterator it(_tree);
    while (*it) {
        QTreeWidgetItem *item = *it;
        bool visible = shouldShowItem(item, searchText, showSelectedOnly);
        item->setHidden(!visible);
        
        // Ensure parents are visible if children are
        if (visible) {
            QTreeWidgetItem *p = item->parent();
            while (p) {
                p->setHidden(false);
                // p->setExpanded(true); // Don't auto-expand everything, just show path
                p = p->parent();
            }
        }
        ++it;
    }
}

void SignalSelectorDialog::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) return;

    _tree->blockSignals(true);
    Qt::CheckState state = item->checkState(0);

    // 1. Propagate DOWN to all children recursively
    auto propagateDown = [&](auto self, QTreeWidgetItem *parentItem, Qt::CheckState checkState) -> void {
        for (int i = 0; i < parentItem->childCount(); ++i) {
            QTreeWidgetItem *child = parentItem->child(i);
            child->setCheckState(0, checkState);
            self(self, child, checkState);
        }
    };
    propagateDown(propagateDown, item, state);

    // 2. Propagate UP to all parents recursively and update visual highlighting
    auto propagateUp = [&](auto self, QTreeWidgetItem *childItem) -> void {
        QTreeWidgetItem *parent = childItem->parent();
        if (!parent) return;

        int checkedCount = 0;
        int partiallyCheckedCount = 0;
        for (int i = 0; i < parent->childCount(); ++i) {
            Qt::CheckState childState = parent->child(i)->checkState(0);
            if (childState == Qt::Checked) checkedCount++;
            else if (childState == Qt::PartiallyChecked) partiallyCheckedCount++;
        }

        if (checkedCount == parent->childCount()) {
            parent->setCheckState(0, Qt::Checked);
        } else if (checkedCount > 0 || partiallyCheckedCount > 0) {
            parent->setCheckState(0, Qt::PartiallyChecked);
        } else {
            parent->setCheckState(0, Qt::Unchecked);
        }

        // Apply visual highlighting (bold font) if item or any child is checked
        QFont font = parent->font(0);
        bool shouldBeBold = (parent->checkState(0) != Qt::Unchecked);
        font.setBold(shouldBeBold);
        parent->setFont(0, font);

        self(self, parent);
    };

    // Also update the current item's font
    QFont itemFont = item->font(0);
    itemFont.setBold(state != Qt::Unchecked);
    item->setFont(0, itemFont);

    propagateUp(propagateUp, item);

    _tree->blockSignals(false);
}

bool SignalSelectorDialog::shouldShowItem(QTreeWidgetItem *item, const QString &searchText, bool showSelectedOnly)
{
    // If it's a signal (has data)
    void* sigPtr = item->data(0, Qt::UserRole).value<void*>();
    if (sigPtr) {
        bool matchSearch = searchText.isEmpty() || item->text(0).contains(searchText, Qt::CaseInsensitive);
        bool matchSelected = !showSelectedOnly || (item->checkState(0) == Qt::Checked);
        return matchSearch && matchSelected;
    }

    // For containers, we check if they have any visible children
    for (int i = 0; i < item->childCount(); ++i) {
        if (shouldShowItem(item->child(i), searchText, showSelectedOnly)) {
            return true;
        }
    }

    // Also show if the group itself matches the search
    if (!searchText.isEmpty() && item->text(0).contains(searchText, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

void SignalSelectorDialog::applyTheme(ThemeManager::Theme theme)
{
    bool isDark = (theme == ThemeManager::Dark);
    
    // Revert dialog-level background styling to keep original view
    this->setStyleSheet("");

    if (isDark) {
        // Targeted styling for tree indicators (CAN messages and signals)
        QString treeStyle = 
            "QTreeView::indicator {"
            "  width: 14px;"
            "  height: 14px;"
            "  border: 1px solid #999;"
            "  border-radius: 2px;"
            "  background-color: #333;"
            "}"
            "QTreeView::indicator:checked {"
            "  background-color: #27ae60;"
            "  border: 1px solid #2ecc71;"
            "}"
            "QTreeView::indicator:unchecked {"
            "  background-color: #333;"
            "}"
            "QTreeView::indicator:unchecked:hover {"
            "  border: 1px solid #ccc;"
            "}"
            "QTreeView::indicator:indeterminate {"
            "  background-color: #555;"
            "}";

        // Targeted styling for the standalone "Show selection only" checkbox
        QString checkStyle = 
            "QCheckBox::indicator {"
            "  width: 14px;"
            "  height: 14px;"
            "  border: 1px solid #999;"
            "  border-radius: 2px;"
            "  background-color: #333;"
            "}"
            "QCheckBox::indicator:checked {"
            "  background-color: #27ae60;"
            "  border: 1px solid #2ecc71;"
            "}"
            "QCheckBox::indicator:unchecked:hover {"
            "  border: 1px solid #ccc;"
            "}";

        _tree->setStyleSheet(treeStyle);
        _showSelectedOnly->setStyleSheet(checkStyle);
    }
 else {
        // Restore standard Light Mode styling
        _tree->setStyleSheet("");
        _showSelectedOnly->setStyleSheet("");
    }
}
