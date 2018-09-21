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
    _backend(backend)
{
    ui->setupUi(this);

    connect(ui->singleSendButton, SIGNAL(released()), this, SLOT(sendRawMessage()));
    connect(ui->repeatSendButton, SIGNAL(toggled(bool)), this, SLOT(sendRepeatMessage(bool)));

    connect(ui->spinBox_RepeatRate, SIGNAL(valueChanged(int)), this, SLOT(changeRepeatRate(int)));

    // Timer for repeating messages
    repeatmsg_timer = new QTimer(this);
    connect(repeatmsg_timer, SIGNAL(timeout()), this, SLOT(sendRawMessage()));


    // TODO: Grey out checkboxes that are invalid depending on DLC spinbox state
}

RawTxWindow::~RawTxWindow()
{
    delete ui;
}

void RawTxWindow::changeRepeatRate(int ms)
{
    repeatmsg_timer->setInterval(ms);
}

void RawTxWindow::sendRepeatMessage(bool enable)
{
    if(enable)
    {
        repeatmsg_timer->start(ui->spinBox_RepeatRate->value());
    }
    else
    {
        repeatmsg_timer->stop();
    }
}

void RawTxWindow::sendRawMessage()
{
    CanMessage msg;

    bool en_extended = ui->checkBox_IsExtended->isChecked();
    bool en_rtr = ui->checkBox_IsRTR->isChecked();
    bool en_errorframe = ui->checkBox_IsErrorFrame->isChecked();

    uint8_t data_int[8];

    data_int[0] = ui->fieldByte0->text().toUpper().toInt(NULL, 16);
    data_int[1] = ui->fieldByte1->text().toUpper().toInt(NULL, 16);
    data_int[2] = ui->fieldByte2->text().toUpper().toInt(NULL, 16);
    data_int[3] = ui->fieldByte3->text().toUpper().toInt(NULL, 16);
    data_int[4] = ui->fieldByte4->text().toUpper().toInt(NULL, 16);
    data_int[5] = ui->fieldByte5->text().toUpper().toInt(NULL, 16);
    data_int[6] = ui->fieldByte6->text().toUpper().toInt(NULL, 16);
    data_int[7] = ui->fieldByte7->text().toUpper().toInt(NULL, 16);

    uint32_t address = ui->fieldAddress->text().toUpper().toInt(NULL, 16);

    // If address is beyond std address namespace, force extended
    if(address > 0x7ff)
    {
        en_extended = true;
        ui->checkBox_IsExtended->setChecked(true);
    }

    uint8_t dlc = ui->fieldDLC->text().toUpper().toInt(NULL, 16);

    msg.setData(data_int[0],data_int[1],data_int[2],data_int[3],data_int[4],data_int[5],data_int[6],data_int[7]);
    msg.setId(address);
    msg.setLength(dlc);

    msg.setExtended(en_extended);
    msg.setRTR(en_rtr);
    msg.setErrorFrame(en_errorframe);


    foreach (CanInterfaceId ifid, _backend.getInterfaceList()) {
        CanInterface *intf = _backend.getInterfaceById(ifid);
        intf->sendMessage(msg);


        char outmsg[256];
        snprintf(outmsg, 256, "Send [%s] to %d on port %s [ext=%u rtr=%u err=%u]",
                 msg.getDataHexString().toLocal8Bit().constData(), msg.getId(), _backend.getInterfaceById(ifid)->getName().toLocal8Bit().constData(),
                 msg.isExtended(), msg.isRTR(), msg.isErrorFrame());
        log_info(outmsg);
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
