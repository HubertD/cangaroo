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

#include "LogWindow.h"
#include "ui_LogWindow.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <core/Backend.h>
#include <core/LogModel.h>

LogWindow::LogWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::LogWindow),
    _backend(&backend)
{
    ui->setupUi(this);

    connect(&backend.getLogModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));

    ui->treeView->setModel(&backend.getLogModel());

    _scroll_timer.setInterval(1);
    _scroll_timer.setSingleShot(true);
    connect(&_scroll_timer,SIGNAL(timeout()),this,SLOT(_scroll_timer_timeout()));
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

    _scroll_timer.start();
    //ui->treeView->scrollToBottom();
}

void LogWindow::_scroll_timer_timeout()
{
    ui->treeView->scrollToBottom();
}

void LogWindow::on_btnExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Logs"), "", tr("Log Files (*.log);;Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    LogModel &model = _backend->getLogModel();
    int rows = model.rowCount(QModelIndex());
    
    // Header
    out << model.headerData(LogModel::column_time, Qt::Horizontal, Qt::DisplayRole).toString() << "\t"
        << model.headerData(LogModel::column_level, Qt::Horizontal, Qt::DisplayRole).toString() << "\t"
        << model.headerData(LogModel::column_text, Qt::Horizontal, Qt::DisplayRole).toString() << "\n";
    out << "------------------------------------------------------------\n";

    for (int i = 0; i < rows; ++i) {
        QString time = model.data(model.index(i, LogModel::column_time, QModelIndex()), Qt::DisplayRole).toString();
        QString level = model.data(model.index(i, LogModel::column_level, QModelIndex()), Qt::DisplayRole).toString();
        QString text = model.data(model.index(i, LogModel::column_text, QModelIndex()), Qt::DisplayRole).toString();
        out << time << "\t" << level << "\t" << text << "\n";
    }

    file.close();
}

void LogWindow::on_btnClear_clicked()
{
    _backend->getLogModel().clear();
}
