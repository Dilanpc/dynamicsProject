#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>

#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QListWidget>

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

    QGridLayout* layout;
    Arduino* arduino;

private:
    QTimer* readTimer;
    QElapsedTimer* clock;

    QWidget* centralWidget;

    QSlider* slider;
    QLineEdit* speed;
    QPushButton* btnDir;

    Grapher* posGraph; // Position
    Grapher* velGraph; // Speed
    Grapher* accGraph; // Acceleration

    void serialPortSelection();
    void mainInterface();


private slots:
    void readPos();
    void readVel();
    void readAcc();

    void changeDir();




private: // Sections
    class SerialPortSelection : public QWidget
    {

    public:
        SerialPortSelection(MainWindow* parent);
        QVBoxLayout* layout;

        QLabel* title;
        QListWidget* list;
    };
};





#endif // MAINWINDOW_H





