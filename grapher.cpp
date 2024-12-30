#include "grapher.h"

Grapher::Grapher(QObject *parent, int amount)
    : QObject(parent),
    series(new QLineSeries*[amount]),
    chart(new QChart()),
    axisX(new QValueAxis()),
    axisY(new QValueAxis()),
    chartView(new QChartView())
{

    for (int i = 0; i < amount; ++i) {
        series[i] = new QLineSeries();
        chart->addSeries(series[i]);
    }

    chart->setTitle("Title");
    // Style
    chart->setBackgroundBrush(QBrush(QColor(45,45,45)));
    chart->setTitleBrush(QBrush(QColor(0xee, 0xee, 0xee)));
    QFont titleFont("Arial", 14, QFont::Bold);
    chart->setTitleFont(titleFont);
    chart->legend()->setLabelColor(Qt::white);
    //chart->setAnimationOptions(QChart::SeriesAnimations);

    axisX->setTitleText("X axis");
    axisX->setTitleBrush(QBrush(QColor(0xee, 0xee, 0xee)));
    axisX->setLabelFormat("%i");
    axisX->setLabelsColor(QColor(0xee, 0xee, 0xee));
    chart->addAxis(axisX, Qt::AlignBottom); 

    axisY->setTitleText("Y axis");
    axisY->setTitleBrush(QBrush(QColor(0xee, 0xee, 0xee)));
    axisY->setLabelFormat("%i");
    axisY->setLabelsColor(QColor(0xee, 0xee, 0xee));
    chart->addAxis(axisY, Qt::AlignLeft);

    for (int i = 0; i < amount; ++i) {
        series[i]->attachAxis(axisX);
        series[i]->attachAxis(axisY);
    }

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);


}

Grapher::~Grapher()
{
    delete[] series;
}
