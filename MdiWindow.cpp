#include "MdiWindow.h"

#include <QDomDocument>
#include <QMdiSubWindow>

#include "Backend.h"

MdiWindow::MdiWindow(QWidget *parent) : QWidget(parent)
{

}

bool MdiWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    root.setAttribute("title", windowTitle());
    return true;
}

