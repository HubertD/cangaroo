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

#include <core/Backend.h>
#include <core/ConfigurableWidget.h>
#include <core/MeasurementSetup.h>

namespace Ui {
class RawTxWindow;
}

class QDomDocument;
class QDomElement;

class CanDbMessage;
class QTableWidgetItem;

class RawTxWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit RawTxWindow(QWidget *parent, Backend &backend);
    ~RawTxWindow();

    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);


public slots:
    void setMessage(const CanMessage &msg, const QString &name, CanInterfaceId interfaceId, CanDbMessage *dbMsg = nullptr);

signals:
    void messageUpdated(const CanMessage &msg);

private slots:
    void reflash_can_msg(void);
    void changeDLC();
    void updateCapabilities();
    void changeRepeatRate(int ms);
    void sendRepeatMessage(bool enable);
    void disableTxWindow(int disable);
    void refreshInterfaces();
    void sendRawMessage();

    void fieldAddress_textChanged(QString str);

    void sendstate_timer_timeout();

    void repeatmsg_timer_timeout();

    void onSignalTableItemChanged(QTableWidgetItem *item);

    void syncSignalsToMessage();

private:
    Ui::RawTxWindow *ui;
    Backend &_backend;
    QTimer *repeatmsg_timer;
    QTimer *sendstate_timer;

    CanMessage _can_msg;
    CanInterface *_intf;

    void hideFDFields();
    void showFDFields();

    void updateSignalTable();

    CanInterfaceId _slavedInterfaceId;
    bool _is_setting_message;
    bool _is_updating_table;
    CanDbMessage *_currentDbMsg;

};
