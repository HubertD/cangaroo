#include "GraphWindow.h"
#include "ui_GraphWindow.h"

#include <QDomDocument>

#include <Backend.h>

GraphWindow::GraphWindow(QWidget *parent, Backend &backend) :
    MdiWindow(parent),
    ui(new Ui::GraphWindow),
    _backend(backend)
{
    ui->setupUi(this);
}

GraphWindow::~GraphWindow()
{
    delete ui;
}

bool GraphWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!MdiWindow::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "GraphWindow");
    return true;
}

bool GraphWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!MdiWindow::loadXML(backend, el)) { return false; }
    return true;
}
