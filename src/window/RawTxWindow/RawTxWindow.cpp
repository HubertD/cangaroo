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

#include "RawTxWindow.h"
#include "ui_RawTxWindow.h"

#include <QDomDocument>
#include <QTimer>
#include <core/Backend.h>
#include <driver/CanInterface.h>

RawTxWindow::RawTxWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::RawTxWindow),
    _backend(backend),
    _currentDbMsg(nullptr)
{
    ui->setupUi(this);

    connect(ui->singleSendButton, SIGNAL(released()), this, SLOT(sendRawMessage()));
    connect(ui->repeatSendButton, SIGNAL(toggled(bool)), this, SLOT(sendRepeatMessage(bool)));

    connect(ui->spinBox_RepeatRate, SIGNAL(valueChanged(int)), this, SLOT(changeRepeatRate(int)));

    connect(ui->fieldAddress, SIGNAL(textChanged(QString)), this, SLOT(fieldAddress_textChanged(QString)));

    // connect(ui->comboBoxInterface, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCapabilities()));
    connect(ui->checkbox_FD, SIGNAL(stateChanged(int)), this, SLOT(updateCapabilities()));

    connect(&backend, SIGNAL(beginMeasurement()),  this, SLOT(refreshInterfaces()));

    connect(&backend, SIGNAL(endMeasurement()),  this, SLOT(refreshInterfaces()));

    // Timer for repeating messages
    repeatmsg_timer = new QTimer(this);
    repeatmsg_timer->setTimerType(Qt::PreciseTimer);
    connect(repeatmsg_timer, SIGNAL(timeout()), this, SLOT(repeatmsg_timer_timeout()));

    sendstate_timer = new QTimer(this);
    sendstate_timer->setInterval(100);
    connect(sendstate_timer, SIGNAL(timeout()), this, SLOT(sendstate_timer_timeout()));

    // TODO: Grey out checkboxes that are invalid depending on DLC spinbox state
    //connect(ui->fieldDLC, SIGNAL(valueChanged(int)), this, SLOT(changeDLC(int)));
    connect(ui->comboBoxDLC, SIGNAL(currentIndexChanged(int)), this, SLOT(changeDLC()));

    _is_setting_message = false;
    _is_updating_table = false;

    QList<QLineEdit*> lines = this->findChildren<QLineEdit*>();
    QRegularExpression hex8("^[0-9A-Fa-f]{0,8}$");
    QRegularExpression hex2("^[0-9A-Fa-f]{0,2}$");

    for (QLineEdit *l : lines) {
        l->setInputMask("");
        if (l == ui->fieldAddress) {
            l->setValidator(new QRegularExpressionValidator(hex8, this));
        } else if (l->objectName().startsWith("fieldByte")) {
            l->setValidator(new QRegularExpressionValidator(hex2, this));
        }

        connect(l, &QLineEdit::textChanged, this, [l, this](const QString &text){
            if (text != text.toUpper()) {
                int pos = l->cursorPosition();
                l->setText(text.toUpper());
                l->setCursorPosition(pos);
            }
            this->reflash_can_msg();
        });
    }
    QList<QCheckBox*> checks = this->findChildren<QCheckBox*>();
    for (QCheckBox *c : checks) {
        connect(c, SIGNAL(stateChanged(int)), this, SLOT(reflash_can_msg()));
    }
    connect(ui->comboBoxDLC, SIGNAL(currentIndexChanged(int)), this, SLOT(reflash_can_msg()));

    connect(ui->comboBoxDLC, SIGNAL(currentIndexChanged(int)), this, SLOT(reflash_can_msg()));

    connect(ui->tableSignals, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onSignalTableItemChanged(QTableWidgetItem*)));

    ui->fieldAddress->clear();
    this->setEnabled(false);
}

RawTxWindow::~RawTxWindow()
{
    delete ui;
}

void RawTxWindow::changeDLC()
{
    ui->fieldByte0_0->setEnabled(true);
    ui->fieldByte1_0->setEnabled(true);
    ui->fieldByte2_0->setEnabled(true);
    ui->fieldByte3_0->setEnabled(true);
    ui->fieldByte4_0->setEnabled(true);
    ui->fieldByte5_0->setEnabled(true);
    ui->fieldByte6_0->setEnabled(true);
    ui->fieldByte7_0->setEnabled(true);

    ui->fieldByte0_1->setEnabled(true);
    ui->fieldByte1_1->setEnabled(true);
    ui->fieldByte2_1->setEnabled(true);
    ui->fieldByte3_1->setEnabled(true);
    ui->fieldByte4_1->setEnabled(true);
    ui->fieldByte5_1->setEnabled(true);
    ui->fieldByte6_1->setEnabled(true);
    ui->fieldByte7_1->setEnabled(true);

    ui->fieldByte0_2->setEnabled(true);
    ui->fieldByte1_2->setEnabled(true);
    ui->fieldByte2_2->setEnabled(true);
    ui->fieldByte3_2->setEnabled(true);
    ui->fieldByte4_2->setEnabled(true);
    ui->fieldByte5_2->setEnabled(true);
    ui->fieldByte6_2->setEnabled(true);
    ui->fieldByte7_2->setEnabled(true);

    ui->fieldByte0_3->setEnabled(true);
    ui->fieldByte1_3->setEnabled(true);
    ui->fieldByte2_3->setEnabled(true);
    ui->fieldByte3_3->setEnabled(true);
    ui->fieldByte4_3->setEnabled(true);
    ui->fieldByte5_3->setEnabled(true);
    ui->fieldByte6_3->setEnabled(true);
    ui->fieldByte7_3->setEnabled(true);

    ui->fieldByte0_4->setEnabled(true);
    ui->fieldByte1_4->setEnabled(true);
    ui->fieldByte2_4->setEnabled(true);
    ui->fieldByte3_4->setEnabled(true);
    ui->fieldByte4_4->setEnabled(true);
    ui->fieldByte5_4->setEnabled(true);
    ui->fieldByte6_4->setEnabled(true);
    ui->fieldByte7_4->setEnabled(true);

    ui->fieldByte0_5->setEnabled(true);
    ui->fieldByte1_5->setEnabled(true);
    ui->fieldByte2_5->setEnabled(true);
    ui->fieldByte3_5->setEnabled(true);
    ui->fieldByte4_5->setEnabled(true);
    ui->fieldByte5_5->setEnabled(true);
    ui->fieldByte6_5->setEnabled(true);
    ui->fieldByte7_5->setEnabled(true);

    ui->fieldByte0_6->setEnabled(true);
    ui->fieldByte1_6->setEnabled(true);
    ui->fieldByte2_6->setEnabled(true);
    ui->fieldByte3_6->setEnabled(true);
    ui->fieldByte4_6->setEnabled(true);
    ui->fieldByte5_6->setEnabled(true);
    ui->fieldByte6_6->setEnabled(true);
    ui->fieldByte7_6->setEnabled(true);

    ui->fieldByte0_7->setEnabled(true);
    ui->fieldByte1_7->setEnabled(true);
    ui->fieldByte2_7->setEnabled(true);
    ui->fieldByte3_7->setEnabled(true);
    ui->fieldByte4_7->setEnabled(true);
    ui->fieldByte5_7->setEnabled(true);
    ui->fieldByte6_7->setEnabled(true);
    ui->fieldByte7_7->setEnabled(true);

    uint8_t dlc = ui->comboBoxDLC->currentData().toUInt();

    // If DLC > 8, must be FD
    if(dlc > 8 && !ui->checkbox_FD->isChecked())
    {
        ui->checkbox_FD->blockSignals(true);
        ui->checkbox_FD->setChecked(true);
        ui->checkbox_FD->blockSignals(false);
    }

    switch(dlc)
    {
        case 0:
            ui->fieldByte0_0->setEnabled(false);
            //fallthrough
        case 1:
            ui->fieldByte1_0->setEnabled(false);
            //fallthrough

        case 2:
            ui->fieldByte2_0->setEnabled(false);
            //fallthrough

        case 3:
            ui->fieldByte3_0->setEnabled(false);
            //fallthrough

        case 4:
            ui->fieldByte4_0->setEnabled(false);
            //fallthrough

        case 5:
            ui->fieldByte5_0->setEnabled(false);
            //fallthrough

        case 6:
            ui->fieldByte6_0->setEnabled(false);
            //fallthrough

        case 7:
            ui->fieldByte7_0->setEnabled(false);
            //fallthrough

        case 8:
            ui->fieldByte0_1->setEnabled(false);
            ui->fieldByte1_1->setEnabled(false);
            ui->fieldByte2_1->setEnabled(false);
            ui->fieldByte3_1->setEnabled(false);
            //fallthrough
        case 12:
            ui->fieldByte4_1->setEnabled(false);
            ui->fieldByte5_1->setEnabled(false);
            ui->fieldByte6_1->setEnabled(false);
            ui->fieldByte7_1->setEnabled(false);
            //fallthrough
    case 16:
        ui->fieldByte0_2->setEnabled(false);
        ui->fieldByte1_2->setEnabled(false);
        ui->fieldByte2_2->setEnabled(false);
        ui->fieldByte3_2->setEnabled(false);
        //fallthrough
    case 20:
        ui->fieldByte4_2->setEnabled(false);
        ui->fieldByte5_2->setEnabled(false);
        ui->fieldByte6_2->setEnabled(false);
        ui->fieldByte7_2->setEnabled(false);
        //fallthrough
    case 24:
        ui->fieldByte0_3->setEnabled(false);
        ui->fieldByte1_3->setEnabled(false);
        ui->fieldByte2_3->setEnabled(false);
        ui->fieldByte3_3->setEnabled(false);
        ui->fieldByte4_3->setEnabled(false);
        ui->fieldByte5_3->setEnabled(false);
        ui->fieldByte6_3->setEnabled(false);
        ui->fieldByte7_3->setEnabled(false);
        //fallthrough
    case 32:
        ui->fieldByte0_4->setEnabled(false);
        ui->fieldByte1_4->setEnabled(false);
        ui->fieldByte2_4->setEnabled(false);
        ui->fieldByte3_4->setEnabled(false);
        ui->fieldByte4_4->setEnabled(false);
        ui->fieldByte5_4->setEnabled(false);
        ui->fieldByte6_4->setEnabled(false);
        ui->fieldByte7_4->setEnabled(false);

        ui->fieldByte0_5->setEnabled(false);
        ui->fieldByte1_5->setEnabled(false);
        ui->fieldByte2_5->setEnabled(false);
        ui->fieldByte3_5->setEnabled(false);
        ui->fieldByte4_5->setEnabled(false);
        ui->fieldByte5_5->setEnabled(false);
        ui->fieldByte6_5->setEnabled(false);
        ui->fieldByte7_5->setEnabled(false);
        //fallthrough
    case 48:
        ui->fieldByte0_6->setEnabled(false);
        ui->fieldByte1_6->setEnabled(false);
        ui->fieldByte2_6->setEnabled(false);
        ui->fieldByte3_6->setEnabled(false);
        ui->fieldByte4_6->setEnabled(false);
        ui->fieldByte5_6->setEnabled(false);
        ui->fieldByte6_6->setEnabled(false);
        ui->fieldByte7_6->setEnabled(false);

        ui->fieldByte0_7->setEnabled(false);
        ui->fieldByte1_7->setEnabled(false);
        ui->fieldByte2_7->setEnabled(false);
        ui->fieldByte3_7->setEnabled(false);
        ui->fieldByte4_7->setEnabled(false);
        ui->fieldByte5_7->setEnabled(false);
        ui->fieldByte6_7->setEnabled(false);
        ui->fieldByte7_7->setEnabled(false);

    }
//    repeatmsg_timer->setInterval(ms);
}

void RawTxWindow::updateCapabilities()
{
    // By default BRS should be available
    CanInterface *intf = _backend.getInterfaceById(_slavedInterfaceId);

    if(intf == NULL)
    {
        return;
    }

    _intf = intf; // store current interface

    int idx_restore = ui->comboBoxDLC->currentIndex();

    // If CANFD is available
    if(intf->getCapabilities() & intf->capability_canfd)
        {
            ui->comboBoxDLC->clear();
            ui->comboBoxDLC->addItem("0", 0);
            ui->comboBoxDLC->addItem("1", 1);
            ui->comboBoxDLC->addItem("2", 2);
            ui->comboBoxDLC->addItem("3", 3);
            ui->comboBoxDLC->addItem("4", 4);
            ui->comboBoxDLC->addItem("5", 5);
            ui->comboBoxDLC->addItem("6", 6);
            ui->comboBoxDLC->addItem("7", 7);
            ui->comboBoxDLC->addItem("8", 8);
            ui->comboBoxDLC->addItem("12", 12);
            ui->comboBoxDLC->addItem("16", 16);
            ui->comboBoxDLC->addItem("20", 20);
            ui->comboBoxDLC->addItem("24", 24);
            ui->comboBoxDLC->addItem("32", 32);
            ui->comboBoxDLC->addItem("48", 48);
            ui->comboBoxDLC->addItem("64", 64);

            // Restore previous selected DLC if available
            if(idx_restore > 1 && idx_restore < ui->comboBoxDLC->count())
                ui->comboBoxDLC->setCurrentIndex(idx_restore);

            ui->checkbox_FD->setDisabled(0);

            // Enable BRS if this is an FD frame
            if(ui->checkbox_FD->isChecked())
            {
                // Enable BRS if FD enabled
                ui->checkbox_BRS->setDisabled(0);

                // Disable RTR if FD enabled
                ui->checkBox_IsRTR->setDisabled(1);
                ui->checkBox_IsRTR->setChecked(false);
            }
            else
            {
                // Disable BRS if FD disabled
                ui->checkbox_BRS->setDisabled(1);
                ui->checkbox_BRS->setChecked(false);

                // Enable RTR if FD disabled
                ui->checkBox_IsRTR->setDisabled(0);

            }
            showFDFields();
        }
        else
        {
            // CANFD not available
            ui->comboBoxDLC->clear();
            ui->comboBoxDLC->addItem("0", 0);
            ui->comboBoxDLC->addItem("1", 1);
            ui->comboBoxDLC->addItem("2", 2);
            ui->comboBoxDLC->addItem("3", 3);
            ui->comboBoxDLC->addItem("4", 4);
            ui->comboBoxDLC->addItem("5", 5);
            ui->comboBoxDLC->addItem("6", 6);
            ui->comboBoxDLC->addItem("7", 7);
            ui->comboBoxDLC->addItem("8", 8);

            // Restore previous selected DLC if available
            if(idx_restore > 1 && idx_restore < ui->comboBoxDLC->count())
                ui->comboBoxDLC->setCurrentIndex(idx_restore);

            // Unset/disable FD / BRS checkboxes
            ui->checkbox_FD->setDisabled(1);
            ui->checkbox_BRS->setDisabled(1);
            ui->checkbox_FD->setChecked(false);
            ui->checkbox_BRS->setChecked(false);

            // Enable RTR (could be disabled by FD checkbox being set)
            ui->checkBox_IsRTR->setDisabled(0);

            hideFDFields();

    }
}

void RawTxWindow::changeRepeatRate(int ms)
{
    if(ms)
        repeatmsg_timer->setInterval(ms);
    else
        ui->spinBox_RepeatRate->setValue(1);
}

void RawTxWindow::sendRepeatMessage(bool enable)
{
    if(enable)
    {
        reflash_can_msg();

        char outmsg[256];
        _intf = _backend.getInterfaceById(_slavedInterfaceId);
        snprintf(outmsg, 256, "Send [%s] to %d on port %s [ext=%u rtr=%u err=%u fd=%u brs=%u]",
                 _can_msg.getDataHexString().toLocal8Bit().constData(), _can_msg.getId(), _intf->getName().toLocal8Bit().constData(),
                 _can_msg.isExtended(), _can_msg.isRTR(), _can_msg.isErrorFrame(), _can_msg.isFD(), _can_msg.isBRS());
        log_info(outmsg);

        repeatmsg_timer->start(ui->spinBox_RepeatRate->value());
        ui->spinBox_RepeatRate->setEnabled(false);
        ui->singleSendButton->setEnabled(false);
        // ui->comboBoxInterface->setEnabled(false);
    }
    else
    {
        repeatmsg_timer->stop();
        ui->spinBox_RepeatRate->setEnabled(true);
        ui->singleSendButton->setEnabled(true);
        // ui->comboBoxInterface->setEnabled(true);
    }
}

void RawTxWindow::repeatmsg_timer_timeout()
{
    if(!_intf->isOpen())
    {
        log_error(_intf->getName() + " not Open!");
        return;
    }
    _intf->sendMessage(_can_msg);
}

void RawTxWindow::disableTxWindow(int disable)
{
    if(disable)
    {
        if(ui->repeatSendButton->isChecked())
            ui->repeatSendButton->toggle();
        this->setDisabled(1);
    }
    else
    {
        // Only enable if an interface is present
        if(_backend.getInterfaceList().count() > 0)
            this->setDisabled(0);
        else
            this->setDisabled(1);
    }
}

void RawTxWindow::refreshInterfaces()
{
    // No longer have a combo box to refresh.
    // The Generator window handles interface selection.
    _is_setting_message = true;
    this->setEnabled(_backend.isMeasurementRunning());
    updateCapabilities();
    _is_setting_message = false;
}

void RawTxWindow::reflash_can_msg()
{
    if (_is_setting_message) return;

    bool en_extended = ui->checkBox_IsExtended->isChecked();
    bool en_rtr = ui->checkBox_IsRTR->isChecked();
    bool en_brs = ui->checkbox_BRS->isChecked();
    bool en_fd = ui->checkbox_FD->isChecked();

    uint8_t data_int[64];
    int data_ctr = 0;

    data_int[data_ctr++] = ui->fieldByte0_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_0->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_1->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_1->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_2->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_3->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_3->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_4->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_5->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_5->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_6->text().toUpper().toInt(NULL, 16);

    data_int[data_ctr++] = ui->fieldByte0_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_7->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_7->text().toUpper().toInt(NULL, 16);

    uint32_t address = ui->fieldAddress->text().toUpper().toUInt(NULL, 16);

    // If address is beyond std address namespace, force extended
    if(address > 0x7ff)
    {
        en_extended = true;
        ui->checkBox_IsExtended->setChecked(true);
    }

    // If address is larger than max for extended, clip
    if(address >= 0x1FFFFFFF)
    {
        address = address & 0x1FFFFFFF;
        ui->fieldAddress->setText(QString::number( address, 16 ).toUpper());
    }

    uint8_t dlc =ui->comboBoxDLC->currentData().toUInt();

    // If DLC > 8, must be FD
    if(dlc > 8 && !ui->checkbox_FD->isChecked())
    {
        en_fd = true;
        ui->checkbox_FD->blockSignals(true);
        ui->checkbox_FD->setChecked(true);
        ui->checkbox_FD->blockSignals(false);
    }

    _can_msg.setId(address);
    _can_msg.setInterfaceId(_slavedInterfaceId);
    _can_msg.setLength(dlc);

    _can_msg.setExtended(en_extended);
    _can_msg.setRTR(en_rtr);
    _can_msg.setErrorFrame(false);

    _can_msg.setBRS(en_brs);
    _can_msg.setFD(en_fd);

    // Set payload data
    for(int i=0; i<dlc; i++)
    {
        _can_msg.setDataAt(i, data_int[i]);
    }

    _can_msg.setRX(false);
    _can_msg.setShow(true);

    struct timeval tv;
    gettimeofday(&tv,NULL);
    _can_msg.setTimestamp(tv);

    if (!_is_setting_message) {
        emit messageUpdated(_can_msg);
        updateSignalTable();
    }
}

void RawTxWindow::updateSignalTable()
{
    _is_updating_table = true;
    ui->tableSignals->setRowCount(0);
    if (!_currentDbMsg) {
        _is_updating_table = false;
        return;
    }

    CanDbSignalList sigList = _currentDbMsg->getSignals();
    ui->tableSignals->setRowCount(sigList.size());

    for (int i = 0; i < sigList.size(); ++i) {
        CanDbSignal *sig = sigList[i];
        double val = sig->extractPhysicalFromMessage(_can_msg);
        
        QTableWidgetItem *nameItem = new QTableWidgetItem(sig->name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->tableSignals->setItem(i, 0, nameItem);
        
        QTableWidgetItem *valItem = new QTableWidgetItem(QString::number(val, 'f', 2));
        valItem->setData(Qt::UserRole, QVariant::fromValue((void*)sig));
        ui->tableSignals->setItem(i, 1, valItem);
        
        QTableWidgetItem *unitItem = new QTableWidgetItem(sig->getUnit());
        unitItem->setFlags(unitItem->flags() & ~Qt::ItemIsEditable);
        ui->tableSignals->setItem(i, 2, unitItem);
    }
    _is_updating_table = false;
}

void RawTxWindow::onSignalTableItemChanged(QTableWidgetItem *item)
{
    if (_is_updating_table || !item) return;

    if (item->column() == 1) {
        CanDbSignal *sig = (CanDbSignal*)item->data(Qt::UserRole).value<void*>();
        if (sig) {
            bool ok;
            double physicalValue = item->text().toDouble(&ok);
            if (ok) {
                sig->applyPhysicalToMessage(physicalValue, _can_msg);
                
                _is_setting_message = true;
                
                for (int i = 0; i < 64; ++i) {
                    QString fieldName = QString("fieldByte%1_%2").arg(i % 8).arg(i / 8);
                    QLineEdit *field = this->findChild<QLineEdit*>(fieldName);
                    if (field) {
                        field->setText(QString("%1").arg(_can_msg.getByte(i), 2, 16, QChar('0')).toUpper());
                    }
                }
                
                _is_setting_message = false;
                
                emit messageUpdated(_can_msg);
            }
        }
    }
}

void RawTxWindow::syncSignalsToMessage()
{
    if (!_currentDbMsg) {
        return;
    }

    const int rows = ui->tableSignals->rowCount();
    for (int i = 0; i < rows; ++i) {
        QTableWidgetItem *valItem = ui->tableSignals->item(i, 1);
        if (!valItem) {
            continue;
        }
        CanDbSignal *sig = static_cast<CanDbSignal *>(valItem->data(Qt::UserRole).value<void *>());
        if (!sig) {
            continue;
        }
        bool ok = false;
        const double physicalValue = valItem->text().toDouble(&ok);
        if (ok) {
            sig->applyPhysicalToMessage(physicalValue, _can_msg);
        }
    }
}

void RawTxWindow::sendRawMessage()
{
    reflash_can_msg();
    // DBC signal values override hex fields for defined signals (correct bit packing).
    syncSignalsToMessage();

    CanInterface *intf = _backend.getInterfaceById(_slavedInterfaceId);
    if(!intf) return;
    if(!intf->isOpen())
    {
        log_error(intf->getName() + " not Open!");
        return;
    }
    _can_msg.setInterfaceId(intf->getId());
    intf->sendMessage(_can_msg);

    char outmsg[256];
    snprintf(outmsg, 256, "Send [%s] to %d on port %s [ext=%u rtr=%u err=%u fd=%u brs=%u]",
             _can_msg.getDataHexString().toLocal8Bit().constData(), _can_msg.getId(), intf->getName().toLocal8Bit().constData(),
             _can_msg.isExtended(), _can_msg.isRTR(), _can_msg.isErrorFrame(), _can_msg.isFD(), _can_msg.isBRS());
    log_info(outmsg);

}

void RawTxWindow::sendstate_timer_timeout()
{
    CanInterface *intf = _backend.getInterfaceById(_slavedInterfaceId);
    if(intf && intf->isOpen())
    {
        if(intf->getState() == CanInterface::state_tx_fail || intf->getState() == CanInterface::state_tx_success)
            ui->label_3->setText(intf->getName() + ' ' + intf->getStateText());
        else
            ui->label_3->setText("");
    }
}

bool RawTxWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "RawTxWindow");
    return true;
}

bool RawTxWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el)) { return false; }
    return true;
}

void RawTxWindow::hideFDFields()
{

    ui->label_col21->hide();
    ui->label_col22->hide();
    ui->label_col23->hide();
    ui->label_col24->hide();
    ui->label_col25->hide();
    ui->label_col26->hide();
    ui->label_col27->hide();
    ui->label_col28->hide();

    ui->label_pay2->hide();
    ui->label_pay3->hide();
    ui->label_pay4->hide();
    ui->label_pay5->hide();
    ui->label_pay6->hide();
    ui->label_pay7->hide();
    ui->label_pay8->hide();

    ui->fieldByte0_1->hide();
    ui->fieldByte1_1->hide();
    ui->fieldByte2_1->hide();
    ui->fieldByte3_1->hide();
    ui->fieldByte4_1->hide();
    ui->fieldByte5_1->hide();
    ui->fieldByte6_1->hide();
    ui->fieldByte7_1->hide();

    ui->fieldByte0_2->hide();
    ui->fieldByte1_2->hide();
    ui->fieldByte2_2->hide();
    ui->fieldByte3_2->hide();
    ui->fieldByte4_2->hide();
    ui->fieldByte5_2->hide();
    ui->fieldByte6_2->hide();
    ui->fieldByte7_2->hide();

    ui->fieldByte0_3->hide();
    ui->fieldByte1_3->hide();
    ui->fieldByte2_3->hide();
    ui->fieldByte3_3->hide();
    ui->fieldByte4_3->hide();
    ui->fieldByte5_3->hide();
    ui->fieldByte6_3->hide();
    ui->fieldByte7_3->hide();

    ui->fieldByte0_4->hide();
    ui->fieldByte1_4->hide();
    ui->fieldByte2_4->hide();
    ui->fieldByte3_4->hide();
    ui->fieldByte4_4->hide();
    ui->fieldByte5_4->hide();
    ui->fieldByte6_4->hide();
    ui->fieldByte7_4->hide();

    ui->fieldByte0_5->hide();
    ui->fieldByte1_5->hide();
    ui->fieldByte2_5->hide();
    ui->fieldByte3_5->hide();
    ui->fieldByte4_5->hide();
    ui->fieldByte5_5->hide();
    ui->fieldByte6_5->hide();
    ui->fieldByte7_5->hide();

    ui->fieldByte0_6->hide();
    ui->fieldByte1_6->hide();
    ui->fieldByte2_6->hide();
    ui->fieldByte3_6->hide();
    ui->fieldByte4_6->hide();
    ui->fieldByte5_6->hide();
    ui->fieldByte6_6->hide();
    ui->fieldByte7_6->hide();

    ui->fieldByte0_7->hide();
    ui->fieldByte1_7->hide();
    ui->fieldByte2_7->hide();
    ui->fieldByte3_7->hide();
    ui->fieldByte4_7->hide();
    ui->fieldByte5_7->hide();
    ui->fieldByte6_7->hide();
    ui->fieldByte7_7->hide();
}


void RawTxWindow::showFDFields()
{

    ui->label_col21->show();
    ui->label_col22->show();
    ui->label_col23->show();
    ui->label_col24->show();
    ui->label_col25->show();
    ui->label_col26->show();
    ui->label_col27->show();
    ui->label_col28->show();

    ui->label_pay2->show();
    ui->label_pay3->show();
    ui->label_pay4->show();
    ui->label_pay5->show();
    ui->label_pay6->show();
    ui->label_pay7->show();
    ui->label_pay8->show();


    ui->fieldByte0_1->show();
    ui->fieldByte1_1->show();
    ui->fieldByte2_1->show();
    ui->fieldByte3_1->show();
    ui->fieldByte4_1->show();
    ui->fieldByte5_1->show();
    ui->fieldByte6_1->show();
    ui->fieldByte7_1->show();

    ui->fieldByte0_2->show();
    ui->fieldByte1_2->show();
    ui->fieldByte2_2->show();
    ui->fieldByte3_2->show();
    ui->fieldByte4_2->show();
    ui->fieldByte5_2->show();
    ui->fieldByte6_2->show();
    ui->fieldByte7_2->show();

    ui->fieldByte0_3->show();
    ui->fieldByte1_3->show();
    ui->fieldByte2_3->show();
    ui->fieldByte3_3->show();
    ui->fieldByte4_3->show();
    ui->fieldByte5_3->show();
    ui->fieldByte6_3->show();
    ui->fieldByte7_3->show();

    ui->fieldByte0_4->show();
    ui->fieldByte1_4->show();
    ui->fieldByte2_4->show();
    ui->fieldByte3_4->show();
    ui->fieldByte4_4->show();
    ui->fieldByte5_4->show();
    ui->fieldByte6_4->show();
    ui->fieldByte7_4->show();

    ui->fieldByte0_5->show();
    ui->fieldByte1_5->show();
    ui->fieldByte2_5->show();
    ui->fieldByte3_5->show();
    ui->fieldByte4_5->show();
    ui->fieldByte5_5->show();
    ui->fieldByte6_5->show();
    ui->fieldByte7_5->show();

    ui->fieldByte0_6->show();
    ui->fieldByte1_6->show();
    ui->fieldByte2_6->show();
    ui->fieldByte3_6->show();
    ui->fieldByte4_6->show();
    ui->fieldByte5_6->show();
    ui->fieldByte6_6->show();
    ui->fieldByte7_6->show();

    ui->fieldByte0_7->show();
    ui->fieldByte1_7->show();
    ui->fieldByte2_7->show();
    ui->fieldByte3_7->show();
    ui->fieldByte4_7->show();
    ui->fieldByte5_7->show();
    ui->fieldByte6_7->show();
    ui->fieldByte7_7->show();
}


void RawTxWindow::fieldAddress_textChanged(QString str)
{
    uint32_t address = ui->fieldAddress->text().toUpper().toUInt(NULL, 16);

    // If address is beyond std address namespace, force extended
    if(address > 0x7ff || str.length() > 3)
    {
        ui->checkBox_IsExtended->setChecked(true);
    }
    else
    {
        ui->checkBox_IsExtended->setChecked(false);
    }

}

void RawTxWindow::setMessage(const CanMessage &msg, const QString &name, CanInterfaceId interfaceId, CanDbMessage *dbMsg)
{
    _is_setting_message = true;
    
    this->setEnabled(true);
    Q_UNUSED(name);

    _slavedInterfaceId = interfaceId;
    updateCapabilities();
    this->setEnabled(true);

    _currentDbMsg = dbMsg;

    bool isExtended = msg.isExtended();
    bool isFD = msg.isFD();
    bool isBRS = msg.isBRS();
    int dlc = msg.getLength();

    ui->fieldAddress->setText(QString("%1").arg(msg.getId(), 0, 16).toUpper());

    if (dbMsg) {
        isExtended = (dbMsg->getRaw_id() & 0x80000000) != 0;
        int dlcCode = dbMsg->getDlc();
        if (dlcCode <= 8) {
            dlc = dlcCode;
        } else {
            isFD = true;
            static const int dlcToLen[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
            if (dlcCode < 16) {
                dlc = dlcToLen[dlcCode];
            }
        }
    }

    ui->checkBox_IsExtended->setChecked(isExtended);
    ui->checkbox_FD->setChecked(isFD);
    ui->checkbox_BRS->setChecked(isBRS);

    int dlc_index = ui->comboBoxDLC->findData(dlc);
    if (dlc_index != -1) {
        ui->comboBoxDLC->setCurrentIndex(dlc_index);
    }
    
    // Load data bytes into fields
    for (int i = 0; i < 64; ++i) {
        QString fieldName = QString("fieldByte%1_%2").arg(i % 8).arg(i / 8);
        QLineEdit *field = this->findChild<QLineEdit*>(fieldName);
        if (field) {
            field->setText(QString("%1").arg(msg.getByte(i), 2, 16, QChar('0')).toUpper());
        }
    }

    updateSignalTable();

    _can_msg = msg;

    _is_setting_message = false;
}
