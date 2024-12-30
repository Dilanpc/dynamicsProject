#ifndef GRAPHER_H
#define GRAPHER_H

#include <QObject>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class Grapher : public QObject
{
    Q_OBJECT
public:
    explicit Grapher(QObject *parent, int amount = 1);
    ~Grapher();

    QLineSeries** series;
    QChart* chart;
    QValueAxis* axisX;
    QValueAxis* axisY;
    QChartView* chartView;


};

#endif // GRAPHER_H
