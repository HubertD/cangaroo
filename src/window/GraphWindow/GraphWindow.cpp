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

#include "GraphWindow.h"
#include "ui_GraphWindow.h"

#include <QDomDocument>

#include <core/Backend.h>
#include <QtCharts/QChartView>

#define NUM_GRAPH_POINTS 20

GraphWindow::GraphWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::GraphWindow),
    _backend(backend)
{
    ui->setupUi(this);


    data_series = new QLineSeries();

    for(uint32_t i=0; i<NUM_GRAPH_POINTS; i++)
    {
        data_series->append(i, i);
    }

    data_chart = new QChart();
    data_chart->legend()->hide();
    data_chart->addSeries(data_series);
    data_chart->createDefaultAxes();
    data_chart->setTitle("Simple line chart example");



    ui->chartView->setChart(data_chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

//    connect(ui->buttonTest, SIGNAL(released()), this, SLOT(testAddData()));

}

void GraphWindow::testAddData(qreal new_yval)
{
    QLineSeries* serbuf = new QLineSeries();

    // Start autorange at first point
    qreal ymin = data_series->at(1).y();
    qreal ymax = ymin;

    // Copy all points but first one
    for(uint32_t i=1; i < data_series->count(); i++)
    {
        serbuf->append(data_series->at(i).x()-1, data_series->at(i).y());

        // Autoranging
        if(data_series->at(i).y() < ymin)
            ymin = data_series->at(i).y();
        if(data_series->at(i).y() > ymax)
            ymax = data_series->at(i).y();
    }

    // Apply Y margin and set range
    ymin -= 1;
    ymax += 1;
    data_chart->axisY()->setRange(ymin, ymax);

    // Add new point in
    serbuf->append(serbuf->points().at(serbuf->count()-1).x() + 1, new_yval);
    testcount++;

    // Replace data
    data_series->replace(serbuf->points());

    delete serbuf;
}

GraphWindow::~GraphWindow()
{
    delete ui;
    delete data_chart;
    delete data_series;
}

bool GraphWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "GraphWindow");
    return true;
}

bool GraphWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el)) { return false; }
    return true;
}
