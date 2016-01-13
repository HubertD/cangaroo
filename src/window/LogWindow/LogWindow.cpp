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

#include "LogWindow.h"
#include "ui_LogWindow.h"

#include <QDomDocument>
#include <Backend.h>

LogWindow::LogWindow(QWidget *parent, Backend &backend) :
    MdiWindow(parent),
    ui(new Ui::LogWindow)
{
    ui->setupUi(this);
    connect(&backend, SIGNAL(onLogMessage(QDateTime,log_level_t,QString)), this, SLOT(onLogMessage(QDateTime,log_level_t,QString)));
}

LogWindow::~LogWindow()
{
    delete ui;
}

bool LogWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!MdiWindow::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "LogWindow");
    return true;
}

bool LogWindow::loadXML(Backend &backend, QDomElement &el)
{
    return MdiWindow::loadXML(backend, el);
}

void LogWindow::onLogMessage(const QDateTime dt, const log_level_t level, const QString msg)
{
    (void) dt;
    (void) level;
    ui->loglist->addItem(msg);
    ui->loglist->scrollToBottom();
}
