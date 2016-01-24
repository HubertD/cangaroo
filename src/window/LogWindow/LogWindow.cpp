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
#include <core/Backend.h>
#include <core/LogModel.h>

LogWindow::LogWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::LogWindow)
{
    connect(&backend.getLogModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));

    ui->setupUi(this);
    ui->treeView->setModel(&backend.getLogModel());
}

LogWindow::~LogWindow()
{
    delete ui;
}

bool LogWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "LogWindow");
    return true;
}

bool LogWindow::loadXML(Backend &backend, QDomElement &el)
{
    return ConfigurableWidget::loadXML(backend, el);
}

void LogWindow::rowsInserted(const QModelIndex &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;
    ui->treeView->scrollToBottom();
}
