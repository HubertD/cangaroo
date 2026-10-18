/*
  Copyright (c) 2026 Jayachandran Dharuman
  This file is part of CANgaroo.
*/

#include "ConditionalLoggingDialog.h"
#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>
#include <core/CanDb.h>
#include <core/CanDbMessage.h>
#include <core/CanDbSignal.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>

ConditionalLoggingDialog::ConditionalLoggingDialog(Backend &backend, QWidget *parent)
    : QDialog(parent), _backend(backend)
{
    setWindowTitle(tr("Conditional Logging Configuration"));
    setMinimumSize(600, 600);
    setupUi();
    populateSignals();

    connect(this, &ConditionalLoggingDialog::finished, [this]() {
        disconnect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &ConditionalLoggingDialog::applyTheme);
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &ConditionalLoggingDialog::applyTheme);
    applyTheme(ThemeManager::instance().currentTheme());

    ConditionalLoggingManager *mgr = _backend.getConditionalLoggingManager();
    _enabledCheckBox->setChecked(false); // Force disabled by default per user request
    _andLogicCheckBox->setChecked(mgr->useAndLogic());
    _logFileEdit->setText(mgr->getLogFilePath());

    // Load hierarchical selection (signals for logging)
    QList<CanDbSignal*> filterSignalSet = mgr->getLogSignals();

    _signalsTree->blockSignals(true);
    QTreeWidgetItemIterator it(_signalsTree);
    while (*it) {
        QTreeWidgetItem *item = *it;
        QString type = item->data(0, Qt::UserRole + 1).toString();
        void *data = item->data(0, Qt::UserRole).value<void*>();

        if (type == "signal") {
            if (filterSignalSet.contains((CanDbSignal*)data)) item->setCheckState(0, Qt::Checked);
        }
        ++it;
    }
    _signalsTree->blockSignals(false);

    // Load conditions
    foreach (const LoggingCondition &cond, mgr->getConditions()) {
        addCondition();
        QTreeWidgetItem *item = _conditionsTree->topLevelItem(_conditionsTree->topLevelItemCount() - 1);
        QComboBox *sigC = qobject_cast<QComboBox*>(_conditionsTree->itemWidget(item, 0));
        QComboBox *opC = qobject_cast<QComboBox*>(_conditionsTree->itemWidget(item, 1));
        QLineEdit *valE = qobject_cast<QLineEdit*>(_conditionsTree->itemWidget(item, 2));

        if (sigC && opC && valE) {
            for (int i=0; i<sigC->count(); ++i) {
                if (sigC->itemData(i).value<void*>() == cond.signal) {
                    sigC->setCurrentIndex(i);
                    break;
                }
            }
            opC->setCurrentIndex(opC->findData((int)cond.op));
            valE->setText(QString::number(cond.threshold));
        }
    }
}

ConditionalLoggingDialog::~ConditionalLoggingDialog()
{
}

void ConditionalLoggingDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Conditions Section
    mainLayout->addWidget(new QLabel(tr("Conditions (Trigger Logging)"), this));
    _conditionsTree = new QTreeWidget(this);
    _conditionsTree->setColumnCount(3);
    _conditionsTree->setHeaderLabels({tr("Signal"), tr("Operator"), tr("Threshold")});
    _conditionsTree->header()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(_conditionsTree);

    QHBoxLayout *condButtonsLayout = new QHBoxLayout();
    QPushButton *btnAddCond = new QPushButton(tr("Add Condition"), this);
    QPushButton *btnRemCond = new QPushButton(tr("Remove Condition"), this);
    condButtonsLayout->addWidget(btnAddCond);
    condButtonsLayout->addWidget(btnRemCond);
    condButtonsLayout->addStretch();
    _andLogicCheckBox = new QCheckBox(tr("Use AND logic for all conditions"), this);
    _andLogicCheckBox->setChecked(true);
    condButtonsLayout->addWidget(_andLogicCheckBox);
    mainLayout->addLayout(condButtonsLayout);

    // Hierarchical Selection Section
    mainLayout->addWidget(new QLabel(tr("CAN Selection (Network / Message / Signal)"), this));
    _signalsTree = new QTreeWidget(this);
    _signalsTree->setColumnCount(1);
    _signalsTree->setHeaderLabels({tr("Hierarchy")});
    mainLayout->addWidget(_signalsTree);

    // File Section
    QHBoxLayout *fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel(tr("Log File Path (Optional for filtering):"), this));
    _logFileEdit = new QLineEdit(this);
    _logFileEdit->setReadOnly(true);
    fileLayout->addWidget(_logFileEdit);
    QPushButton *btnBrowse = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(btnBrowse);
    mainLayout->addLayout(fileLayout);

    _enabledCheckBox = new QCheckBox(tr("Enable Conditional Logging (File Output)"), this);
    mainLayout->addWidget(_enabledCheckBox);

    // Dialog Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnOk = new QPushButton(tr("OK"), this);
    QPushButton *btnCancel = new QPushButton(tr("Cancel"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnOk);
    buttonLayout->addWidget(btnCancel);
    mainLayout->addLayout(buttonLayout);

    connect(btnAddCond, &QPushButton::clicked, this, &ConditionalLoggingDialog::addCondition);
    connect(btnRemCond, &QPushButton::clicked, this, &ConditionalLoggingDialog::removeCondition);
    connect(btnBrowse, &QPushButton::clicked, this, &ConditionalLoggingDialog::selectLogFile);
    connect(btnOk, &QPushButton::clicked, this, &ConditionalLoggingDialog::onAccept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(_signalsTree, &QTreeWidget::itemChanged, this, &ConditionalLoggingDialog::onItemChanged);
}

void ConditionalLoggingDialog::populateSignals()
{
    _signalsTree->blockSignals(true);
    _signalsTree->clear();
    
    foreach (MeasurementNetwork *network, _backend.getSetup().getNetworks()) {
        QTreeWidgetItem *networkItem = new QTreeWidgetItem(_signalsTree);
        networkItem->setText(0, network->name());
        networkItem->setCheckState(0, Qt::Unchecked);
        networkItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)network));
        networkItem->setData(0, Qt::UserRole + 1, "network");

        foreach (pCanDb db, network->_canDbs) {
            foreach (CanDbMessage *msg, db->getMessageList().values()) {
                QTreeWidgetItem *msgItem = new QTreeWidgetItem(networkItem);
                msgItem->setText(0, msg->getName());
                msgItem->setCheckState(0, Qt::Unchecked);
                msgItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)msg));
                msgItem->setData(0, Qt::UserRole + 1, "message");

                foreach (CanDbSignal *sig, msg->getSignals()) {
                    QTreeWidgetItem *sigItem = new QTreeWidgetItem(msgItem);
                    sigItem->setText(0, sig->name());
                    sigItem->setCheckState(0, Qt::Unchecked);
                    sigItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)sig));
                    sigItem->setData(0, Qt::UserRole + 1, "signal");
                }
            }
        }
    }
    _signalsTree->blockSignals(false);
}

void ConditionalLoggingDialog::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) return;

    _signalsTree->blockSignals(true);
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

    _signalsTree->blockSignals(false);
}

void ConditionalLoggingDialog::addCondition()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(_conditionsTree);
    
    QComboBox *signalCombo = new QComboBox(this);
    QList<pCanDb> allDbs;
    foreach (MeasurementNetwork *network, _backend.getSetup().getNetworks()) {
        allDbs.append(network->_canDbs);
    }
    foreach (pCanDb db, allDbs) {
        foreach (CanDbMessage *msg, db->getMessageList().values()) {
            foreach (CanDbSignal *sig, msg->getSignals()) {
                signalCombo->addItem(sig->name(), QVariant::fromValue((void*)sig));
            }
        }
    }
    _conditionsTree->setItemWidget(item, 0, signalCombo);

    QComboBox *opCombo = new QComboBox(this);
    opCombo->addItem(">", (int)ConditionOperator::Greater);
    opCombo->addItem("<", (int)ConditionOperator::Less);
    opCombo->addItem("==", (int)ConditionOperator::Equal);
    opCombo->addItem(">=", (int)ConditionOperator::GreaterEqual);
    opCombo->addItem("<=", (int)ConditionOperator::LessEqual);
    opCombo->addItem("!=", (int)ConditionOperator::NotEqual);
    _conditionsTree->setItemWidget(item, 1, opCombo);

    QLineEdit *valEdit = new QLineEdit("0.0", this);
    _conditionsTree->setItemWidget(item, 2, valEdit);
}

void ConditionalLoggingDialog::removeCondition()
{
    delete _conditionsTree->currentItem();
}

void ConditionalLoggingDialog::selectLogFile()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Select Log File"), "", tr("CSV Files (*.csv)"));
    if (!path.isEmpty()) {
        _logFileEdit->setText(path);
    }
}

#include <QTreeWidgetItemIterator>

void ConditionalLoggingDialog::onAccept()
{
    ConditionalLoggingManager *mgr = _backend.getConditionalLoggingManager();

    QMap<CanDbSignal*, CanInterfaceIdList> signalInterfaces;
    QList<CanDbSignal*> logSignals;

    QTreeWidgetItemIterator it(_signalsTree);
    while (*it) {
        QTreeWidgetItem *item = *it;
        if (item->checkState(0) == Qt::Checked) {
            QString type = item->data(0, Qt::UserRole + 1).toString();
            void *data = item->data(0, Qt::UserRole).value<void*>();
            
            if (type == "signal") {
                CanDbSignal *sig = (CanDbSignal*)data;
                logSignals.append(sig);

                // Find the parent network to get Interface IDs
                QTreeWidgetItem *parent = item->parent(); // message
                if (parent) parent = parent->parent(); // network
                if (parent && parent->data(0, Qt::UserRole + 1).toString() == "network") {
                    MeasurementNetwork *net = (MeasurementNetwork*)parent->data(0, Qt::UserRole).value<void*>();
                    CanInterfaceIdList ifIds;
                    foreach (MeasurementInterface *mi, net->interfaces()) {
                        ifIds.append(mi->canInterface());
                    }
                    signalInterfaces[sig] = ifIds;
                }
            }
        }
        ++it;
    }

    if (_enabledCheckBox->isChecked() && _logFileEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an output file for logging."));
        return;
    }

    QList<LoggingCondition> conditions;
    for (int i = 0; i < _conditionsTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = _conditionsTree->topLevelItem(i);
        QComboBox *sigC = qobject_cast<QComboBox*>(_conditionsTree->itemWidget(item, 0));
        QComboBox *opC = qobject_cast<QComboBox*>(_conditionsTree->itemWidget(item, 1));
        QLineEdit *valE = qobject_cast<QLineEdit*>(_conditionsTree->itemWidget(item, 2));

        if (sigC && opC && valE) {
            LoggingCondition cond;
            cond.signal = (CanDbSignal*)sigC->currentData().value<void*>();
            cond.op = (ConditionOperator)opC->currentData().toInt();
            cond.threshold = valE->text().toDouble();
            conditions.append(cond);
        }
    }

    mgr->setConditions(conditions, _andLogicCheckBox->isChecked());
    mgr->setLogSignals(logSignals);
    mgr->setSignalInterfaces(signalInterfaces);
    mgr->setLogFilePath(_logFileEdit->text());
    mgr->setEnabled(_enabledCheckBox->isChecked());

    accept();
}
void ConditionalLoggingDialog::applyTheme(ThemeManager::Theme theme)
{
    bool isDark = (theme == ThemeManager::Dark);
    
    // Targeted styling for checkboxes and tree indicators in dark mode
    if (isDark) {
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
            "QTreeView::indicator:unchecked:hover {"
            "  border: 1px solid #ccc;"
            "}";

        _enabledCheckBox->setStyleSheet(checkStyle);
        _andLogicCheckBox->setStyleSheet(checkStyle);
        _signalsTree->setStyleSheet(treeStyle);
    } else {
        _enabledCheckBox->setStyleSheet("");
        _andLogicCheckBox->setStyleSheet("");
        _signalsTree->setStyleSheet("");
    }
}
