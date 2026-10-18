#include "TxGeneratorWindow.h"
#include "ui_TxGeneratorWindow.h"
#include "WaveformDialog.h"
#include <QTreeWidgetItem>
#include <QTimer>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSplitter>
#include <core/Backend.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementInterface.h>
#include <driver/CanInterface.h>
#include <driver/CanDriver.h>
#include <core/CanDbMessage.h>
#include <sys/time.h>
#include <cmath>

static const int COL_STATUS   = 0;
static const int COL_ID       = 1;
static const int COL_NAME     = 2;
static const int COL_IFACE    = 3;
static const int COL_DLC      = 4;
static const int COL_INTERVAL = 5;

// UserRole stores _cyclicMessages index for leaf items, -1 for group headers
static const int ROLE_IDX = Qt::UserRole;
// UserRole+1 stores group name for group header items
static const int ROLE_GROUP = Qt::UserRole + 1;

TxGeneratorWindow::TxGeneratorWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::TxGeneratorWindow),
    _backend(backend)
{
    ui->setupUi(this);

    _sendTimer = new QTimer(this);
    _sendTimer->setInterval(10);
    connect(_sendTimer, SIGNAL(timeout()), this, SLOT(onSendTimerTimeout()));
    _sendTimer->start();

    connect(&backend, SIGNAL(onSetupChanged()), this, SLOT(onSetupChanged()));
    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(refreshInterfaces()));
    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(updateMeasurementState()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(refreshInterfaces()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(updateMeasurementState()));

    connect(ui->btnBulkRun, SIGNAL(clicked()), this, SLOT(on_btnBulkRun_clicked()));
    connect(ui->btnBulkStop, SIGNAL(clicked()), this, SLOT(on_btnBulkStop_clicked()));

    ui->btnBulkRun->setStyleSheet("QPushButton { font-weight: bold; } QPushButton:checked { background-color: #28a745; color: white; border: 1px solid #218838; }");
    ui->btnBulkStop->setStyleSheet("QPushButton { font-weight: bold; } QPushButton:checked { background-color: #dc3545; color: white; border: 1px solid #c82333; }");

    connect(ui->treeActive, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(on_treeActive_itemChanged(QTreeWidgetItem*,int)));
    connect(ui->treeAvailable, SIGNAL(itemSelectionChanged()), this, SLOT(on_treeAvailable_itemSelectionChanged()));

    // Context menu for group operations
    ui->treeActive->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeActive, &QTreeWidget::customContextMenuRequested,
            this, &TxGeneratorWindow::onTreeActiveContextMenu);

    _bitMatrixWidget = new BitMatrixWidget(this);
    ui->scrollAreaLayout->setWidgetResizable(false);
    ui->scrollAreaLayout->setWidget(_bitMatrixWidget);

    ui->sliderLayoutZoom->setRange(30, 120);
    ui->sliderLayoutZoom->setValue(50);
    _bitMatrixWidget->setCellSize(50);
    _bitMatrixWidget->setFixedSize(_bitMatrixWidget->sizeHint());

    ui->lineManualId->setInputMask("");
    ui->lineManualId->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9A-Fa-f]{0,8}$"), this));
    connect(ui->lineManualId, &QLineEdit::textChanged, this, [this](const QString &text){
        if (text != text.toUpper()) {
            int cursorPos = ui->lineManualId->cursorPosition();
            ui->lineManualId->setText(text.toUpper());
            ui->lineManualId->setCursorPosition(cursorPos);
        }
    });

    ui->treeActive->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeAvailable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Random Payload button
    _btnRandomPayload = new QPushButton(tr("🎲 Randomize Payload"), this);
    _btnRandomPayload->setToolTip(tr("Randomize data bytes for selected messages"));
    _btnRandomPayload->setStyleSheet("QPushButton { font-weight: bold; background: #6f42c1; color: white; border-radius: 4px; padding: 4px 8px; } QPushButton:hover { background: #5a32a3; }");
    ui->horizontalLayoutActiveControls->insertWidget(2, _btnRandomPayload);
    connect(_btnRandomPayload, &QPushButton::released, this, &TxGeneratorWindow::onRandomPayloadReleased);

    // Waveform button
    _btnWaveform = new QPushButton(tr("∿ Waveform..."), this);
    _btnWaveform->setToolTip(tr("Configure waveform generation for signals in the selected message"));
    _btnWaveform->setStyleSheet("QPushButton { font-weight: bold; background: #17a2b8; color: white; border-radius: 4px; padding: 4px 8px; } QPushButton:hover { background: #138496; } QPushButton:disabled { background: #444; color: #888; }");
    _btnWaveform->setEnabled(false);
    ui->horizontalLayoutActiveControls->insertWidget(3, _btnWaveform);
    connect(_btnWaveform, &QPushButton::released, this, &TxGeneratorWindow::onWaveformButtonReleased);

    // Create Group button
    _btnCreateGroup = new QPushButton(tr("+ Group"), this);
    _btnCreateGroup->setToolTip(tr("Create a new transmission group"));
    _btnCreateGroup->setStyleSheet("QPushButton { font-weight: bold; background: #fd7e14; color: white; border-radius: 4px; padding: 4px 8px; } QPushButton:hover { background: #e06800; }");
    ui->horizontalLayoutActiveControls->insertWidget(4, _btnCreateGroup);
    connect(_btnCreateGroup, &QPushButton::released, this, &TxGeneratorWindow::onCreateGroupReleased);

    srand(time(NULL));

    refreshInterfaces();
    updateMeasurementState();
    populateDbcMessages();
    updateActiveList();
    isLoading = false;
}

TxGeneratorWindow::~TxGeneratorWindow()
{
    delete ui;
}

bool TxGeneratorWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "TxGeneratorWindow");
    return true;
}

bool TxGeneratorWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el)) { return false; }
    return true;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

int TxGeneratorWindow::itemToMsgIndex(QTreeWidgetItem *item) const
{
    if (!item) return -1;
    return item->data(COL_STATUS, ROLE_IDX).toInt();
}

QList<int> TxGeneratorWindow::selectedMsgIndices() const
{
    QList<int> result;
    for (QTreeWidgetItem *item : ui->treeActive->selectedItems()) {
        int idx = itemToMsgIndex(item);
        if (idx >= 0 && idx < _cyclicMessages.size())
            result.append(idx);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Interface / DBC population
// ---------------------------------------------------------------------------

void TxGeneratorWindow::refreshInterfaces()
{
    ui->comboBoxInterface->blockSignals(true);
    ui->comboBoxInterface->clear();

    MeasurementSetup &setup = _backend.getSetup();
    foreach (MeasurementNetwork *network, setup.getNetworks()) {
        foreach (MeasurementInterface *mi, network->interfaces()) {
            CanInterfaceId ifid = mi->canInterface();
            CanInterface *intf = _backend.getInterfaceById(ifid);
            if (intf) {
                QString name = network->name() + ": " + intf->getName();
                ui->comboBoxInterface->addItem(name, QVariant(ifid));
            }
        }
    }
    if (ui->comboBoxInterface->count() > 0 && ui->comboBoxInterface->currentIndex() == -1)
        ui->comboBoxInterface->setCurrentIndex(0);
    ui->comboBoxInterface->blockSignals(false);
    populateDbcMessages();
}

void TxGeneratorWindow::populateDbcMessages()
{
    ui->treeAvailable->clear();

    CanInterfaceId currentId = (CanInterfaceId)ui->comboBoxInterface->currentData().toUInt();
    MeasurementSetup &setup = _backend.getSetup();

    foreach (MeasurementNetwork *network, setup.getNetworks()) {
        bool interfaceMatches = false;
        foreach (MeasurementInterface *mi, network->interfaces()) {
            if (mi->canInterface() == currentId) { interfaceMatches = true; break; }
        }
        if (!interfaceMatches) continue;

        foreach (pCanDb db, network->_canDbs) {
            if (!db) continue;
            CanDbMessageList msgs = db->getMessageList();
            for (auto it = msgs.begin(); it != msgs.end(); ++it) {
                CanDbMessage *dbMsg = *it;
                if (!dbMsg) continue;
                QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeAvailable);
                item->setText(0, "0x" + QString("%1").arg(dbMsg->getRaw_id(), 3, 16, QChar('0')).toUpper());
                item->setText(1, dbMsg->getName());
                item->setData(0, Qt::UserRole, QVariant::fromValue((void*)dbMsg));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Available-list slots
// ---------------------------------------------------------------------------

void TxGeneratorWindow::on_lineEditSearchAvailable_textChanged(const QString &text)
{
    for (int i = 0; i < ui->treeAvailable->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = ui->treeAvailable->topLevelItem(i);
        bool match = item->text(0).contains(text, Qt::CaseInsensitive)
                  || item->text(1).contains(text, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}

void TxGeneratorWindow::on_treeAvailable_itemSelectionChanged()
{
    QTreeWidgetItem *item = ui->treeAvailable->currentItem();
    if (item && _bitMatrixWidget) {
        CanDbMessage *dbMsg = (CanDbMessage*)item->data(0, Qt::UserRole).value<void*>();
        _bitMatrixWidget->setMessage(dbMsg);
    } else if (_bitMatrixWidget) {
        _bitMatrixWidget->setMessage(nullptr);
    }
}

void TxGeneratorWindow::on_sliderLayoutZoom_valueChanged(int value)
{
    if (_bitMatrixWidget) {
        _bitMatrixWidget->setCellSize(value);
        _bitMatrixWidget->setFixedSize(_bitMatrixWidget->sizeHint());
    }
}

void TxGeneratorWindow::on_cbLayoutCompact_toggled(bool checked)
{
    if (_bitMatrixWidget) {
        _bitMatrixWidget->setCompactMode(checked);
        _bitMatrixWidget->setFixedSize(_bitMatrixWidget->sizeHint());
    }
}

// ---------------------------------------------------------------------------
// Add / Remove messages
// ---------------------------------------------------------------------------

void TxGeneratorWindow::on_btnAddToList_released()
{
    QList<QTreeWidgetItem*> selected = ui->treeAvailable->selectedItems();
    if (selected.isEmpty()) {
        QTreeWidgetItem *current = ui->treeAvailable->currentItem();
        if (current) selected.append(current);
    }
    if (selected.isEmpty()) return;

    foreach (QTreeWidgetItem *item, selected) {
        CanDbMessage *dbMsg = (CanDbMessage*)item->data(0, Qt::UserRole).value<void*>();
        if (!dbMsg) continue;

        CyclicMessage cm;
        cm.msg = CanMessage();
        cm.msg.setId(dbMsg->getRaw_id());
        cm.msg.setLength(dbMsg->getDlc());
        cm.msg.setExtended(dbMsg->getRaw_id() > 0x7FF);
        cm.name = dbMsg->getName();
        cm.interval = 100;
        cm.enabled = false;
        cm.lastSent = 0;
        cm.interfaceId = (CanInterfaceId)ui->comboBoxInterface->currentData().toUInt();
        cm.dbMsg = dbMsg;
        _cyclicMessages.append(cm);
    }
    updateActiveList();
    ui->treeActive->scrollToBottom();
}

void TxGeneratorWindow::on_btnAddManual_released()
{
    bool ok;
    uint32_t id = ui->lineManualId->text().toUInt(&ok, 16);
    if (!ok) return;

    CyclicMessage cm;
    cm.msg = CanMessage();
    cm.msg.setId(id);
    cm.msg.setLength(ui->spinManualDlc->value());
    cm.msg.setExtended(id > 0x7FF || ui->lineManualId->text().length() > 3);
    cm.msg.setFD(ui->spinManualDlc->value() > 8);
    cm.name = "Manual";
    cm.interval = 100;
    cm.enabled = false;
    cm.lastSent = 0;
    cm.interfaceId = (CanInterfaceId)ui->comboBoxInterface->currentData().toUInt();
    cm.dbMsg = nullptr;
    _cyclicMessages.append(cm);
    updateActiveList();
    ui->treeActive->scrollToBottom();
}

void TxGeneratorWindow::on_btnRemove_released()
{
    QList<int> rows = selectedMsgIndices();
    if (rows.isEmpty()) return;

    std::sort(rows.begin(), rows.end(), std::greater<int>());
    foreach (int row, rows) {
        if (row >= 0 && row < _cyclicMessages.size())
            _cyclicMessages.removeAt(row);
    }
    updateActiveList();
}

// ---------------------------------------------------------------------------
// Send controls
// ---------------------------------------------------------------------------

void TxGeneratorWindow::on_btnSendOnce_released()
{
    QList<int> rows = selectedMsgIndices();
    if (rows.isEmpty()) {
        int row = itemToMsgIndex(ui->treeActive->currentItem());
        if (row >= 0) rows.append(row);
    }

    foreach (int row, rows) {
        if (row < 0 || row >= _cyclicMessages.size()) continue;
        CyclicMessage &cm = _cyclicMessages[row];
        CanInterface *intf = _backend.getInterfaceById(cm.interfaceId);
        if (intf && intf->isOpen()) {
            cm.msg.setInterfaceId(cm.interfaceId);
            intf->sendMessage(cm.msg);
            if (ui->cbShowInTrace->isChecked()) {
                CanMessage loopback = cm.msg;
                loopback.setRX(false);
                struct timeval tv; gettimeofday(&tv, NULL);
                loopback.setTimestamp(tv);
                emit loopbackFrame(loopback);
            }
        }
    }
}

void TxGeneratorWindow::on_btnBulkRun_clicked()
{
    QList<int> rows = selectedMsgIndices();
    if (rows.isEmpty()) return;
    foreach (int row, rows) {
        _cyclicMessages[row].enabled = true;
        updateRowUI(row);
    }
    ui->btnBulkRun->setChecked(true);
    ui->btnBulkStop->setChecked(false);
}

void TxGeneratorWindow::on_btnBulkStop_clicked()
{
    QList<int> rows = selectedMsgIndices();
    if (rows.isEmpty()) {
        int row = itemToMsgIndex(ui->treeActive->currentItem());
        if (row >= 0 && row < _cyclicMessages.size()) {
            _cyclicMessages[row].enabled = false;
            updateRowUI(row);
        }
    } else {
        foreach (int row, rows) {
            _cyclicMessages[row].enabled = false;
            updateRowUI(row);
        }
    }
    ui->btnBulkRun->setChecked(false);
    ui->btnBulkStop->setChecked(true);
}

void TxGeneratorWindow::on_spinInterval_valueChanged(int i)
{
    QList<int> rows = selectedMsgIndices();
    if (rows.isEmpty()) {
        int row = itemToMsgIndex(ui->treeActive->currentItem());
        if (row >= 0 && row < _cyclicMessages.size()) {
            _cyclicMessages[row].interval = i;
            updateRowUI(row);
        }
    } else {
        foreach (int row, rows) {
            _cyclicMessages[row].interval = i;
            updateRowUI(row);
        }
    }
}

void TxGeneratorWindow::on_comboBoxInterface_currentIndexChanged(int index)
{
    (void)index;
    populateDbcMessages();
    emit interfaceChanged((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
}

void TxGeneratorWindow::on_treeAvailable_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    (void)column;
    on_btnAddToList_released();
}

// ---------------------------------------------------------------------------
// Active-list selection
// ---------------------------------------------------------------------------

void TxGeneratorWindow::on_treeActive_itemSelectionChanged()
{
    isLoading = true;
    QList<int> rows = selectedMsgIndices();

    // Enable waveform button only when exactly one DBC-backed message is selected
    bool waveformOk = (rows.size() == 1 && _cyclicMessages[rows.first()].dbMsg != nullptr);
    _btnWaveform->setEnabled(waveformOk);

    if (!rows.isEmpty()) {
        const CyclicMessage &cm = _cyclicMessages[rows.first()];
        ui->btnBulkRun->blockSignals(true);
        ui->btnBulkStop->blockSignals(true);
        ui->btnBulkRun->setChecked(cm.enabled);
        ui->btnBulkStop->setChecked(!cm.enabled);
        ui->btnBulkRun->blockSignals(false);
        ui->btnBulkStop->blockSignals(false);

        ui->spinInterval->blockSignals(true);
        ui->spinInterval->setValue(cm.interval);
        ui->spinInterval->blockSignals(false);

        emit messageSelected(cm.msg, cm.name, cm.interfaceId, cm.dbMsg);
    } else {
        int row = itemToMsgIndex(ui->treeActive->currentItem());
        if (row >= 0 && row < _cyclicMessages.size()) {
            const CyclicMessage &cm = _cyclicMessages[row];
            ui->btnBulkRun->blockSignals(true);
            ui->btnBulkStop->blockSignals(true);
            ui->btnBulkRun->setChecked(cm.enabled);
            ui->btnBulkStop->setChecked(!cm.enabled);
            ui->btnBulkRun->blockSignals(false);
            ui->btnBulkStop->blockSignals(false);

            ui->spinInterval->blockSignals(true);
            ui->spinInterval->setValue(cm.interval);
            ui->spinInterval->blockSignals(false);

            emit messageSelected(cm.msg, cm.name, cm.interfaceId, cm.dbMsg);
        }
    }
    isLoading = false;
}

void TxGeneratorWindow::on_treeActive_itemChanged(QTreeWidgetItem *item, int column)
{
    int row = itemToMsgIndex(item);
    if (row < 0 || row >= _cyclicMessages.size()) return;

    if (column == COL_INTERVAL) {
        bool ok;
        int interval = item->text(COL_INTERVAL).toInt(&ok);
        if (ok && interval > 0 && _cyclicMessages[row].interval != interval) {
            _cyclicMessages[row].interval = interval;
            if (item->isSelected()) {
                for (QTreeWidgetItem *sel : ui->treeActive->selectedItems()) {
                    if (sel == item) continue;
                    int selRow = itemToMsgIndex(sel);
                    if (selRow >= 0 && selRow < _cyclicMessages.size()) {
                        _cyclicMessages[selRow].interval = interval;
                        updateRowUI(selRow);
                    }
                }
            }
        }
    }
}

void TxGeneratorWindow::on_btnSelectAll_released()
{
    ui->treeActive->selectAll();
}

void TxGeneratorWindow::on_btnClearAll_released()
{
    ui->treeActive->clearSelection();
}

void TxGeneratorWindow::onStatusButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QPoint pos = btn->parentWidget()->mapTo(ui->treeActive->viewport(), btn->pos());
    QTreeWidgetItem *item = ui->treeActive->itemAt(pos);
    if (!item) return;

    int row = itemToMsgIndex(item);
    if (row >= 0 && row < _cyclicMessages.size()) {
        bool targetState = !_cyclicMessages[row].enabled;
        if (item->isSelected()) {
            for (QTreeWidgetItem *sel : ui->treeActive->selectedItems()) {
                int selRow = itemToMsgIndex(sel);
                if (selRow >= 0 && selRow < _cyclicMessages.size()) {
                    _cyclicMessages[selRow].enabled = targetState;
                    updateRowUI(selRow);
                }
            }
        } else {
            _cyclicMessages[row].enabled = targetState;
            updateRowUI(row);
        }
    }
}

// ---------------------------------------------------------------------------
// Group operations
// ---------------------------------------------------------------------------

void TxGeneratorWindow::onCreateGroupReleased()
{
    editGroupMembers(QString());
}

void TxGeneratorWindow::editGroupMembers(const QString &existingGroupName)
{
    const bool isNew = existingGroupName.isEmpty();

    QDialog dlg(this);
    dlg.setWindowTitle(isNew ? tr("Create Group") : tr("Edit Group — %1").arg(existingGroupName));
    dlg.resize(540, 420);
    dlg.setMinimumSize(420, 320);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);

    // ── Group name field ──────────────────────────────────────────────────
    QHBoxLayout *nameRow = new QHBoxLayout;
    nameRow->addWidget(new QLabel(tr("Group name:"), &dlg));
    QLineEdit *nameEdit = new QLineEdit(existingGroupName, &dlg);
    nameEdit->setPlaceholderText(tr("Enter group name…"));
    nameRow->addWidget(nameEdit, 1);
    mainLayout->addLayout(nameRow);

    QFrame *sep = new QFrame(&dlg);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #555;");
    mainLayout->addWidget(sep);

    // ── Two-panel list area ───────────────────────────────────────────────
    QHBoxLayout *listsRow = new QHBoxLayout;

    // Left: Available messages (not in this group)
    QVBoxLayout *availCol = new QVBoxLayout;
    QLabel *lblAvail = new QLabel(tr("Available Messages"), &dlg);
    lblAvail->setStyleSheet("font-weight: bold;");
    availCol->addWidget(lblAvail);
    QListWidget *availList = new QListWidget(&dlg);
    availList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    availList->setAlternatingRowColors(true);
    availList->setToolTip(tr("Double-click or use Add → to move into the group"));
    availCol->addWidget(availList);
    listsRow->addLayout(availCol, 1);

    // Centre: transfer buttons
    QVBoxLayout *btnCol = new QVBoxLayout;
    btnCol->addStretch();
    QPushButton *btnAdd    = new QPushButton(tr("Add →"),    &dlg);
    QPushButton *btnRemove = new QPushButton(tr("← Remove"), &dlg);
    btnAdd->setFixedWidth(95);
    btnRemove->setFixedWidth(95);
    btnAdd->setStyleSheet(
        "QPushButton { background:#28a745; color:white; font-weight:bold; border-radius:4px; padding:4px; }"
        "QPushButton:hover { background:#218838; }");
    btnRemove->setStyleSheet(
        "QPushButton { background:#dc3545; color:white; font-weight:bold; border-radius:4px; padding:4px; }"
        "QPushButton:hover { background:#c82333; }");
    btnCol->addWidget(btnAdd);
    btnCol->addSpacing(6);
    btnCol->addWidget(btnRemove);
    btnCol->addStretch();
    listsRow->addLayout(btnCol);

    // Right: Messages in group
    QVBoxLayout *inGroupCol = new QVBoxLayout;
    QLabel *lblInGroup = new QLabel(tr("In Group"), &dlg);
    lblInGroup->setStyleSheet("font-weight: bold; color: #28a745;");
    inGroupCol->addWidget(lblInGroup);
    QListWidget *inGroupList = new QListWidget(&dlg);
    inGroupList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    inGroupList->setAlternatingRowColors(true);
    inGroupList->setToolTip(tr("Double-click or use ← Remove to move back to available"));
    inGroupCol->addWidget(inGroupList);
    listsRow->addLayout(inGroupCol, 1);

    mainLayout->addLayout(listsRow, 1);

    // Hint label
    QLabel *hint = new QLabel(
        tr("Tip: double-click an entry to move it, or select multiple rows and use the buttons."),
        &dlg);
    hint->setStyleSheet("color: #888; font-size: 10px;");
    hint->setWordWrap(true);
    mainLayout->addWidget(hint);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    mainLayout->addWidget(buttonBox);

    // ── Populate lists ────────────────────────────────────────────────────
    for (int i = 0; i < _cyclicMessages.size(); ++i) {
        const CyclicMessage &cm = _cyclicMessages[i];
        CanInterface *intf = _backend.getInterfaceById(cm.interfaceId);
        QString label = QString("0x%1  %2  [%3]")
            .arg(cm.msg.getId(), 3, 16, QChar('0')).toUpper()
            .arg(cm.name)
            .arg(intf ? intf->getName() : tr("?"));
        // If already in a different group, note it
        if (!cm.groupName.isEmpty() && cm.groupName != existingGroupName)
            label += QString("  ← %1").arg(cm.groupName);

        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, i);

        if (!existingGroupName.isEmpty() && cm.groupName == existingGroupName)
            inGroupList->addItem(item);
        else
            availList->addItem(item);
    }

    // ── Connections ───────────────────────────────────────────────────────
    auto moveToGroup = [&]() {
        const auto sel = availList->selectedItems();
        for (QListWidgetItem *it : sel)
            inGroupList->addItem(availList->takeItem(availList->row(it)));
    };
    auto moveToAvail = [&]() {
        const auto sel = inGroupList->selectedItems();
        for (QListWidgetItem *it : sel)
            availList->addItem(inGroupList->takeItem(inGroupList->row(it)));
    };

    QObject::connect(btnAdd,    &QPushButton::clicked, &dlg, moveToGroup);
    QObject::connect(btnRemove, &QPushButton::clicked, &dlg, moveToAvail);
    QObject::connect(availList,   &QListWidget::itemDoubleClicked, &dlg,
        [&](QListWidgetItem *it){ inGroupList->addItem(availList->takeItem(availList->row(it))); });
    QObject::connect(inGroupList, &QListWidget::itemDoubleClicked, &dlg,
        [&](QListWidgetItem *it){ availList->addItem(inGroupList->takeItem(inGroupList->row(it))); });
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return;

    QString newName = nameEdit->text().trimmed();
    if (newName.isEmpty()) return;

    // Clear old group assignment for any message that was in the existing group
    if (!existingGroupName.isEmpty()) {
        for (CyclicMessage &cm : _cyclicMessages) {
            if (cm.groupName == existingGroupName)
                cm.groupName.clear();
        }
    }

    // Assign the in-group items to the (possibly new or renamed) group
    for (int i = 0; i < inGroupList->count(); ++i) {
        int msgIdx = inGroupList->item(i)->data(Qt::UserRole).toInt();
        if (msgIdx >= 0 && msgIdx < _cyclicMessages.size())
            _cyclicMessages[msgIdx].groupName = newName;
    }

    updateActiveList();
}


void TxGeneratorWindow::deleteGroup(const QString &groupName, bool deleteMessages)
{
    if (deleteMessages) {
        for (int i = _cyclicMessages.size() - 1; i >= 0; --i) {
            if (_cyclicMessages[i].groupName == groupName)
                _cyclicMessages.removeAt(i);
        }
    } else {
        for (CyclicMessage &cm : _cyclicMessages) {
            if (cm.groupName == groupName)
                cm.groupName.clear();
        }
    }
    updateActiveList();
}

void TxGeneratorWindow::assignToGroup(const QList<int> &indices, const QString &groupName)
{
    for (int idx : indices) {
        if (idx >= 0 && idx < _cyclicMessages.size())
            _cyclicMessages[idx].groupName = groupName;
    }
    updateActiveList();
}

void TxGeneratorWindow::setGroupEnabled(const QString &groupName, bool enabled)
{
    for (int i = 0; i < _cyclicMessages.size(); ++i) {
        if (_cyclicMessages[i].groupName == groupName) {
            _cyclicMessages[i].enabled = enabled;
            updateRowUI(i);
        }
    }
}

void TxGeneratorWindow::onTreeActiveContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeActive->itemAt(pos);
    QMenu menu(this);

    if (!item) {
        // Empty area — offer to create a group
        QAction *actCreate = menu.addAction(tr("Create Group..."));
        connect(actCreate, &QAction::triggered, this, [this]() { editGroupMembers(QString()); });
    } else {
        int idx = itemToMsgIndex(item);
        if (idx < 0) {
            // Group header
            QString grpName = item->data(COL_STATUS, ROLE_GROUP).toString();

            QAction *actEdit = menu.addAction(tr("✏ Edit Group Members..."));
            actEdit->setToolTip(tr("Add or remove messages from this group"));
            connect(actEdit, &QAction::triggered, this, [this, grpName]() {
                editGroupMembers(grpName);
            });

            menu.addSeparator();

            QAction *actRunAll = menu.addAction(tr("▶ Run All in Group"));
            connect(actRunAll, &QAction::triggered, this, [this, grpName]() { setGroupEnabled(grpName, true); });

            QAction *actStopAll = menu.addAction(tr("⏹ Stop All in Group"));
            connect(actStopAll, &QAction::triggered, this, [this, grpName]() { setGroupEnabled(grpName, false); });

            menu.addSeparator();

            QAction *actDelKeep = menu.addAction(tr("Delete Group (keep messages)"));
            connect(actDelKeep, &QAction::triggered, this, [this, grpName]() {
                deleteGroup(grpName, false);
            });

            QAction *actDelAll = menu.addAction(tr("Delete Group + Messages"));
            connect(actDelAll, &QAction::triggered, this, [this, grpName]() {
                if (QMessageBox::question(this, tr("Delete Group"),
                        tr("Remove group \"%1\" and all its messages?").arg(grpName))
                        == QMessageBox::Yes)
                    deleteGroup(grpName, true);
            });
        } else {
            // Message item — quick group assignment
            QString currentGroup = _cyclicMessages[idx].groupName;

            QStringList groups;
            for (const CyclicMessage &cm : _cyclicMessages)
                if (!cm.groupName.isEmpty() && !groups.contains(cm.groupName))
                    groups.append(cm.groupName);

            QMenu *assignMenu = menu.addMenu(tr("Assign to Group"));
            for (const QString &g : groups) {
                QAction *act = assignMenu->addAction(g);
                act->setCheckable(true);
                act->setChecked(g == currentGroup);
                connect(act, &QAction::triggered, this, [this, g]() {
                    QList<int> sel = selectedMsgIndices();
                    if (sel.isEmpty()) sel.append(itemToMsgIndex(ui->treeActive->currentItem()));
                    assignToGroup(sel, g);
                });
            }
            assignMenu->addSeparator();
            QAction *actNewGrp = assignMenu->addAction(tr("New Group..."));
            connect(actNewGrp, &QAction::triggered, this, [this]() {
                editGroupMembers(QString());
            });

            if (!currentGroup.isEmpty()) {
                QAction *actRemove = menu.addAction(tr("Remove from Group"));
                connect(actRemove, &QAction::triggered, this, [this, idx]() {
                    QList<int> sel = selectedMsgIndices();
                    if (sel.isEmpty()) sel.append(idx);
                    assignToGroup(sel, QString());
                });
            }
        }
    }

    menu.exec(ui->treeActive->viewport()->mapToGlobal(pos));
}

// ---------------------------------------------------------------------------
// Waveform
// ---------------------------------------------------------------------------

void TxGeneratorWindow::onWaveformButtonReleased()
{
    QList<int> rows = selectedMsgIndices();
    if (rows.size() != 1) return;
    int idx = rows.first();
    if (idx < 0 || idx >= _cyclicMessages.size()) return;

    CyclicMessage &cm = _cyclicMessages[idx];
    if (!cm.dbMsg) return;

    WaveformDialog dlg(cm.dbMsg, cm.waveforms, this);
    if (dlg.exec() == QDialog::Accepted) {
        cm.waveforms = dlg.waveforms();
    }
}

void TxGeneratorWindow::applyWaveforms(CyclicMessage &cm, uint64_t now_ms)
{
    if (!cm.dbMsg || cm.waveforms.isEmpty()) return;

    CanDbSignalList sigList = cm.dbMsg->getSignals();
    double t = static_cast<double>(now_ms);

    for (CanDbSignal *sig : sigList) {
        if (!sig) continue;
        auto it = cm.waveforms.find(sig->name());
        if (it == cm.waveforms.end()) continue;
        const WaveformConfig &wf = it.value();
        if (wf.type == WaveformConfig::None) continue;

        double value = 0.0;

        switch (wf.type) {
        case WaveformConfig::Square: {
            double period = wf.sqTon_ms + wf.sqToff_ms;
            if (period <= 0.0) break;
            double t_cyc = std::fmod(t, period);
            value = (t_cyc < wf.sqTon_ms) ? wf.sqHighVal : wf.sqLowVal;
            break;
        }
        case WaveformConfig::Sine: {
            double T = wf.sinPeriod_ms > 0.0 ? wf.sinPeriod_ms : 1000.0;
            double phase = wf.sinPhase_deg * M_PI / 180.0;
            value = wf.sinOffset + wf.sinAmplitude * std::sin(2.0 * M_PI * t / T + phase);
            break;
        }
        case WaveformConfig::Ramp: {
            double cycle = wf.rampRise_ms + wf.rampHold_ms;
            if (cycle <= 0.0) break;
            double t_cyc = std::fmod(t, cycle);
            if (t_cyc <= wf.rampRise_ms && wf.rampRise_ms > 0.0)
                value = wf.rampStartVal +
                        (wf.rampEndVal - wf.rampStartVal) * (t_cyc / wf.rampRise_ms);
            else
                value = wf.rampEndVal;
            break;
        }
        default: break;
        }

        double minV = sig->getMinimumValue();
        double maxV = sig->getMaximumValue();
        if (maxV > minV) value = qBound(minV, value, maxV);
        sig->applyPhysicalToMessage(value, cm.msg);
    }
}

// ---------------------------------------------------------------------------
// Cyclic send timer
// ---------------------------------------------------------------------------

void TxGeneratorWindow::onSendTimerTimeout()
{
    if (!_backend.isMeasurementRunning()) return;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t now_ms = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);

    for (int i = 0; i < _cyclicMessages.size(); ++i) {
        CyclicMessage &cm = _cyclicMessages[i];
        if (!cm.enabled) continue;
        if (now_ms - cm.lastSent < (uint64_t)cm.interval) continue;

        applyWaveforms(cm, now_ms);

        CanInterface *intf = _backend.getInterfaceById(cm.interfaceId);
        if (intf && intf->isOpen()) {
            cm.msg.setInterfaceId(cm.interfaceId);
            intf->sendMessage(cm.msg);
            if (ui->cbShowInTrace->isChecked()) {
                CanMessage loopback = cm.msg;
                loopback.setRX(false);
                struct timeval tv_loop; gettimeofday(&tv_loop, NULL);
                loopback.setTimestamp(tv_loop);
                emit loopbackFrame(loopback);
            }
            cm.lastSent = now_ms;
        } else {
            QString errorMsg = QString("TxGeneratorWindow: Cyclic - Interface %1 is not open.")
                .arg(intf ? intf->getName() : QString::number(cm.interfaceId));
            log_error(errorMsg);
        }
    }
}

void TxGeneratorWindow::onSetupChanged()
{
    refreshInterfaces();
}

void TxGeneratorWindow::updateMeasurementState()
{
    bool running = _backend.isMeasurementRunning();
    ui->btnSendOnce->setEnabled(running);
    ui->groupBoxActive->setEnabled(running);
    if (!running) stopAll();
}

// ---------------------------------------------------------------------------
// Active list rendering (2-level: group headers + message items)
// ---------------------------------------------------------------------------

void TxGeneratorWindow::updateActiveList()
{
    // Persist selection by message index
    QSet<int> selectedIndices;
    for (QTreeWidgetItem *item : ui->treeActive->selectedItems()) {
        int idx = itemToMsgIndex(item);
        if (idx >= 0) selectedIndices.insert(idx);
    }
    int currentIndex = itemToMsgIndex(ui->treeActive->currentItem());

    ui->treeActive->blockSignals(true);
    ui->treeActive->clear();

    // Build group-header items in order of first occurrence
    QStringList groupOrder;
    QMap<QString, QTreeWidgetItem*> groupItems;

    for (const CyclicMessage &cm : _cyclicMessages) {
        if (!cm.groupName.isEmpty() && !groupOrder.contains(cm.groupName))
            groupOrder.append(cm.groupName);
    }

    for (const QString &grpName : groupOrder) {
        QTreeWidgetItem *grpItem = new QTreeWidgetItem(ui->treeActive);
        grpItem->setData(COL_STATUS, ROLE_IDX, -1);
        grpItem->setData(COL_STATUS, ROLE_GROUP, grpName);
        grpItem->setText(COL_NAME, QString("[ %1 ]").arg(grpName));
        grpItem->setExpanded(true);
        grpItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Style group header row
        for (int c = 0; c < ui->treeActive->columnCount(); ++c) {
            grpItem->setBackground(c, QColor(45, 55, 72));
            grpItem->setForeground(c, QColor(255, 200, 80));
        }
        QFont f = grpItem->font(COL_NAME);
        f.setBold(true); f.setItalic(true);
        grpItem->setFont(COL_NAME, f);

        // Group-level Run/Stop button
        QPushButton *btnGrpRun = new QPushButton("▶ All");
        btnGrpRun->setFixedWidth(55);
        btnGrpRun->setStyleSheet("QPushButton { color: #28a745; font-weight:bold; background:transparent; border:1px solid #28a745; border-radius:3px; } QPushButton:hover { background:#28a745; color:white; }");
        btnGrpRun->setToolTip(tr("Start all messages in group \"%1\"").arg(grpName));
        connect(btnGrpRun, &QPushButton::clicked, this, [this, grpName]() { setGroupEnabled(grpName, true); });
        ui->treeActive->setItemWidget(grpItem, COL_STATUS, btnGrpRun);

        QPushButton *btnGrpStop = new QPushButton("⏹ All");
        btnGrpStop->setFixedWidth(55);
        btnGrpStop->setStyleSheet("QPushButton { color: #dc3545; font-weight:bold; background:transparent; border:1px solid #dc3545; border-radius:3px; } QPushButton:hover { background:#dc3545; color:white; }");
        btnGrpStop->setToolTip(tr("Stop all messages in group \"%1\"").arg(grpName));
        connect(btnGrpStop, &QPushButton::clicked, this, [this, grpName]() { setGroupEnabled(grpName, false); });
        ui->treeActive->setItemWidget(grpItem, COL_ID, btnGrpStop);

        groupItems[grpName] = grpItem;
    }

    // Add message items
    for (int i = 0; i < _cyclicMessages.size(); ++i) {
        const CyclicMessage &cm = _cyclicMessages[i];

        QTreeWidgetItem *item;
        if (!cm.groupName.isEmpty() && groupItems.contains(cm.groupName))
            item = new QTreeWidgetItem(groupItems[cm.groupName]);
        else
            item = new QTreeWidgetItem(ui->treeActive);

        item->setData(COL_STATUS, ROLE_IDX, i);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);

        // Status button
        QPushButton *btnStatus = new QPushButton(cm.enabled ? "⏹" : "▶");
        btnStatus->setToolTip(cm.enabled ? "Stop" : "Start");
        btnStatus->setFixedWidth(40);
        if (cm.enabled)
            btnStatus->setStyleSheet("QPushButton { color:#dc3545; font-weight:bold; background:transparent; border:1px solid #dc3545; border-radius:3px; } QPushButton:hover { background:#dc3545; color:white; }");
        else
            btnStatus->setStyleSheet("QPushButton { color:#28a745; font-weight:bold; background:transparent; border:1px solid #28a745; border-radius:3px; } QPushButton:hover { background:#28a745; color:white; }");
        connect(btnStatus, &QPushButton::clicked, this, &TxGeneratorWindow::onStatusButtonClicked);
        ui->treeActive->setItemWidget(item, COL_STATUS, btnStatus);

        item->setText(COL_ID, "0x" + QString("%1").arg(cm.msg.getId(), 3, 16, QChar('0')).toUpper());
        item->setText(COL_NAME, cm.name);
        CanInterface *intf = _backend.getInterfaceById(cm.interfaceId);
        item->setText(COL_IFACE, intf ? intf->getName() : "Unknown");
        item->setText(COL_DLC, QString::number(cm.msg.getLength()));
        item->setText(COL_INTERVAL, QString::number(cm.interval));

        // Show waveform indicator if any signal has a waveform configured
        bool hasWaveform = false;
        for (const WaveformConfig &wf : cm.waveforms) {
            if (wf.type != WaveformConfig::None) { hasWaveform = true; break; }
        }
        if (hasWaveform)
            item->setText(COL_NAME, cm.name + " ∿");

        if (selectedIndices.contains(i)) item->setSelected(true);
        if (i == currentIndex) ui->treeActive->setCurrentItem(item);
    }

    ui->treeActive->blockSignals(false);
}

void TxGeneratorWindow::updateRowUI(int msgIndex)
{
    if (msgIndex < 0 || msgIndex >= _cyclicMessages.size()) return;

    // Walk all items to find the one with the matching stored index
    std::function<QTreeWidgetItem*(QTreeWidgetItem*)> findItem;
    findItem = [&](QTreeWidgetItem *parent) -> QTreeWidgetItem* {
        int childCount = parent ? parent->childCount() : ui->treeActive->topLevelItemCount();
        for (int i = 0; i < childCount; ++i) {
            QTreeWidgetItem *child = parent ? parent->child(i) : ui->treeActive->topLevelItem(i);
            if (itemToMsgIndex(child) == msgIndex) return child;
            QTreeWidgetItem *found = findItem(child);
            if (found) return found;
        }
        return nullptr;
    };
    QTreeWidgetItem *item = findItem(nullptr);
    if (!item) return;

    const CyclicMessage &cm = _cyclicMessages[msgIndex];

    ui->treeActive->blockSignals(true);

    QPushButton *btnStatus = qobject_cast<QPushButton*>(ui->treeActive->itemWidget(item, COL_STATUS));
    if (btnStatus) {
        btnStatus->setText(cm.enabled ? "⏹" : "▶");
        btnStatus->setToolTip(cm.enabled ? "Stop" : "Start");
        if (cm.enabled)
            btnStatus->setStyleSheet("QPushButton { color:#dc3545; font-weight:bold; background:transparent; border:1px solid #dc3545; border-radius:3px; } QPushButton:hover { background:#dc3545; color:white; }");
        else
            btnStatus->setStyleSheet("QPushButton { color:#28a745; font-weight:bold; background:transparent; border:1px solid #28a745; border-radius:3px; } QPushButton:hover { background:#28a745; color:white; }");
    }

    item->setText(COL_ID, "0x" + QString("%1").arg(cm.msg.getId(), 3, 16, QChar('0')).toUpper());

    bool hasWaveform = false;
    for (const WaveformConfig &wf : cm.waveforms) {
        if (wf.type != WaveformConfig::None) { hasWaveform = true; break; }
    }
    item->setText(COL_NAME, hasWaveform ? cm.name + " ∿" : cm.name);

    CanInterface *intf = _backend.getInterfaceById(cm.interfaceId);
    item->setText(COL_IFACE, intf ? intf->getName() : "Unknown");
    item->setText(COL_DLC, QString::number(cm.msg.getLength()));
    item->setText(COL_INTERVAL, QString::number(cm.interval));

    ui->treeActive->blockSignals(false);
}

void TxGeneratorWindow::updateMessage(const CanMessage &msg)
{
    if (isLoading) return;
    int row = itemToMsgIndex(ui->treeActive->currentItem());
    if (row >= 0 && row < _cyclicMessages.size()) {
        _cyclicMessages[row].msg = msg;
        updateRowUI(row);
    }
}

void TxGeneratorWindow::stopAll()
{
    for (int i = 0; i < _cyclicMessages.size(); ++i) {
        _cyclicMessages[i].enabled = false;
        updateRowUI(i);
    }
    ui->btnBulkRun->setChecked(false);
    ui->btnBulkStop->setChecked(false);
}

QSize TxGeneratorWindow::sizeHint() const
{
    return QSize(1200, 600);
}

void TxGeneratorWindow::onRandomPayloadReleased()
{
    QList<int> rows = selectedMsgIndices();
    if (rows.isEmpty()) {
        int row = itemToMsgIndex(ui->treeActive->currentItem());
        if (row >= 0) rows.append(row);
    }

    foreach (int row, rows) {
        if (row < 0 || row >= _cyclicMessages.size()) continue;
        CyclicMessage &cm = _cyclicMessages[row];
        for (int i = 0; i < cm.msg.getLength(); ++i)
            cm.msg.setDataAt(i, (uint8_t)(rand() % 256));
        updateRowUI(row);

        QTreeWidgetItem *cur = ui->treeActive->currentItem();
        if (cur && itemToMsgIndex(cur) == row)
            emit messageSelected(cm.msg, cm.name, cm.interfaceId, cm.dbMsg);
    }
}
