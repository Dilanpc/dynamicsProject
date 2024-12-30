#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>

#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>


#include <QTimer>
#include <QElapsedTimer>

#include "arduino.h"
#include "grapher.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Arduino* arduino;

private:
    QTimer* readTimer;
    QElapsedTimer* clock;

    QWidget* centralWidget;

    QSlider* slider;
    QLineEdit* speed;
    QPushButton* btnDir;

    Grapher* posGraph;
    Grapher* velGraph;



private slots:
    void readPos();
    void readVel();

    void changeDir();
};
#endif // MAINWINDOW_H
