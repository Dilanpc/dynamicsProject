#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    layout(new QGridLayout()),
    arduino(new Arduino(this)),
    centralWidget(new QWidget(this))
{

    setWindowTitle("Stepping Motor Controller");
    setCentralWidget(centralWidget);
    centralWidget->setLayout(layout);


    SerialPortSelection* serialPortSelection = new SerialPortSelection(this);
    layout->addWidget(serialPortSelection);

    QObject::connect(serialPortSelection->list, &QListWidget::itemClicked, [this, serialPortSelection](QListWidgetItem* item){
        arduino->open(item->text());
        mainInterface();
        serialPortSelection->deleteLater();
    });


}

MainWindow::~MainWindow()
{
    delete clock;
}


void MainWindow::readPos()
{
    posGraph->series[0]->remove(0);
    posGraph->series[0]->append(clock->elapsed(), arduino->position);

    const QPointF first = posGraph->series[0]->at(0);
    const QPointF last = posGraph->series[0]->at(99);

    posGraph->axisX->setRange(first.x(), last.x());
    posGraph->axisY->setRange(last.y() - 500, last.y() + 500);

    readVel();

}

void MainWindow::readVel()
{
    QPointF pos1 = posGraph->series[0]->at(99);
    QPointF pos2 = posGraph->series[0]->at(98);
    QPointF pos3 = posGraph->series[0]->at(97);

    // Using centered differences

    QLineSeries* centered = velGraph->series[1];



    centered->remove(0);

    centered->append(
        pos3.x() + (pos1.x() - pos3.x()) / 2,
        1000 * (pos1.y() - pos3.y()) / (pos1.x() - pos3.x())
        );


    // Deirect
    QLineSeries* direct = velGraph->series[0];

    direct->remove(0);
    direct->append(
        pos2.x() + (pos1.x() - pos2.x()) / 2,
        1000 * (pos1.y() - pos2.y()) / (pos1.x() - pos2.x())
        );


    // Adjust graph for centered
    QPointF first = centered->at(0);
    QPointF last = centered->at(97);

    velGraph->axisX->setRange(first.x(), last.x());
    velGraph->axisY->setRange(last.y() - 480, last.y() + 480);
}


void MainWindow::changeDir()
{
    arduino->write("d");
}




MainWindow::SerialPortSelection::SerialPortSelection(MainWindow* _parent)
    : QWidget(_parent),
    layout(new QVBoxLayout(this)),
    title(new QLabel),
    list(new QListWidget)
{
    title->setText("Seleccione puerto serial");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        list->addItem(info.portName());
    }
    layout->addWidget(list);
}

void MainWindow::mainInterface()
{
    readTimer = new QTimer;
    clock = new QElapsedTimer; // delete manually
    slider = new QSlider(Qt::Horizontal);
    speed = new QLineEdit;
    btnDir = new QPushButton;
    posGraph = new Grapher(this);
    velGraph = new Grapher(this, 2);


    // Slider
    slider->setMinimum(0);
    slider->setMaximum(1080);
    slider->setValue(0);
    layout->addWidget(slider, 0, 0, 2, 1);



    // speed text
    QIntValidator *validator = new QIntValidator(0, 1080, speed); // Rango entre 0 y 100
    speed->setValidator(validator);
    speed->setText("0");
    layout->addWidget(speed, 0, 1);



    // Direction Button
    btnDir->setText("Cambiar dirección");
    connect(btnDir, &QPushButton::clicked, this, &MainWindow::changeDir);
    layout->addWidget(btnDir, 1, 1);


    // Position Chart
    posGraph->series[0]->setName("Posición");
    posGraph->chart->setTitle("Posición vs Tiempo");
    posGraph->axisX->setTitleText("Tiempo");
    posGraph->axisY->setTitleText("Posición");
    posGraph->chart->legend()->hide();

    for (int i = 0; i < 100; ++i) { // Fill with 0
        posGraph->series[0]->append(0,0);
    }

    layout->addWidget(posGraph->chartView, 2, 0, 1, 2);



    // Angular Speed Chart
    velGraph->series[0]->setName("Directa");
    velGraph->series[0]->setColor(QColor(255, 187, 28));

    velGraph->series[1]->setName("Centrada");
    velGraph->series[1]->setColor(QColor(232, 100, 23));

    velGraph->chart->setTitle("Velocidad vs Tiempo");
    velGraph->axisX->setTitleText("Tiempo");
    velGraph->axisY->setTitleText("Velocidad");

    for (int i = 0; i < 98; ++i) { // Fill with 0, 98 because the method centered differences
        velGraph->series[0]->append(0,0);
        velGraph->series[1]->append(0,0);
    }
    velGraph->series[0]->append(0,0); // 99 data for direct method

    layout->addWidget(velGraph->chartView, 3, 0, 1, 2);





    ///////// Connect slider and speed text ///////////
    QObject::connect(slider, &QSlider::valueChanged, this, [this](int value) {
        speed->setText(QString::number(value)); // Actualizar el texto del lineEdit
        arduino->write((QString("s") + QString::number(value) + '\n').toUtf8());
    });

    QObject::connect(speed, &QLineEdit::editingFinished, this, [this]() {
        QString text = speed->text();
        slider->setValue(text.toInt()); // Actualizar el valor del slider
        arduino->write((QString("s") + text + '\n').toUtf8());
    });



    // Read arduino data each 50 milliseconds
    connect(readTimer, &QTimer::timeout, this, &MainWindow::readPos);
    readTimer->start(50);




    clock->start();
}
