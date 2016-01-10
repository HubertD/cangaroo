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

bool MdiWindow::loadXML(Backend &backend, QDomElement &el)
{
    setWindowTitle(el.attribute("title", ""));
    return true;
}
