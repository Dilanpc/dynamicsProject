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
    readAcc();

}

void MainWindow::readVel()
{
    QLineSeries& pos = *posGraph->series[0];

    // Using centered differences

    QLineSeries* centered = velGraph->series[1];


    centered->remove(0);

    centered->append(
        (pos.at(97).x() + pos.at(99).x()) / 2,
        1000 * (pos.at(99).y() - pos.at(97).y()) / (pos.at(99).x() - pos.at(97).x())
        );


    // Deirect
    QLineSeries* direct = velGraph->series[0];

    direct->remove(0);
    direct->append(
        (pos.at(98).x() + pos.at(99).x()) / 2,
        1000 * (pos.at(99).y() - pos.at(98).y()) / (pos.at(99).x() - pos.at(98).x())
        );



    // Average 5
    QLineSeries* avg = velGraph->series[2];

    QPointF point(0,0);
    for (int i = 0; i < 5; ++i) {
        point.setX( point.x() + direct->at(98-i).x() );
        point.setY( point.y() + direct->at(98-i).y() );
    }
    point.setX(point.x() / 5);
    point.setY(point.y() / 5);

    avg->remove(0);
    avg->append(point);


    // Adjust graph for centered
    QPointF first = centered->at(0);    
    QPointF last = centered->at(97);

    velGraph->axisX->setRange(first.x(), last.x());
}


void MainWindow::readAcc()
{
    QPointF velCen1 = velGraph->series[1]->at(97);
    QPointF velCen3 = velGraph->series[1]->at(95);

    QPointF velDir1 = velGraph->series[0]->at(98);
    QPointF velDir2 = velGraph->series[0]->at(97);


    // Deirect
    QLineSeries* direct = accGraph->series[0];

    direct->remove(0);
    direct->append(
        (velDir2.x() + velDir1.x()) / 2, // Average time
        1000 * (velDir1.y() - velDir2.y()) / (velDir1.x() - velDir2.x())
        );


    // Using centered differences

    QLineSeries* centered = accGraph->series[1];

    centered->remove(0);

    centered->append(
        velCen3.x() + (velCen1.x() - velCen3.x()) / 2,
        1000 * (velCen1.y() - velCen3.y()) / (velCen1.x() - velCen3.x())
        );



    // Adjust graph for centered
    QPointF first = centered->at(0);
    QPointF last = centered->at(95);

    accGraph->axisX->setRange(first.x(), last.x());

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
    btnSave = new QPushButton;
    posGraph = new Grapher(this);
    velGraph = new Grapher(this, 3);
    accGraph = new Grapher(this, 2);

    // Layout configuration
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 2);
    QGridLayout* optionsLayout = new QGridLayout;
    layout->addLayout(optionsLayout, 0, 0, 2, 2);


    // Slider
    slider->setMinimum(0);
    slider->setMaximum(1080);
    slider->setValue(0);
    optionsLayout->addWidget(slider, 0, 0, 3, 1);



    // speed text
    QIntValidator *validator = new QIntValidator(0, 1080, speed); // Rango entre 0 y 100
    speed->setValidator(validator);
    speed->setText("0");
    optionsLayout->addWidget(speed, 0, 1);



    // Direction Button
    btnDir->setText("Cambiar dirección");
    connect(btnDir, &QPushButton::clicked, this, &MainWindow::changeDir);
    optionsLayout->addWidget(btnDir, 1, 1);

    // Save Button
    btnSave->setText("Guardar últimos 100");
    //connect(btnSave, &QPushButton::Clicked, this, &MainWindow::saveData);
    optionsLayout->addWidget(btnSave, 2, 1);

    // Position Chart
    posGraph->series[0]->setName("Posición");
    posGraph->chart->setTitle("Posición vs Tiempo");
    posGraph->axisX->setTitleText("Tiempo");
    posGraph->axisY->setTitleText("Posición");
    posGraph->chart->legend()->hide();

    for (int i = 0; i < 100; ++i) { // Fill with 0
        posGraph->series[0]->append(0,0);
    }

    layout->addWidget(posGraph->chartView, 0, 2, 2, 2);



    // Angular Speed Chart
    velGraph->series[0]->setName("Directa");
    velGraph->series[0]->setColor(QColor(255, 187, 28));

    velGraph->series[1]->setName("Centrada");
    velGraph->series[1]->setColor(QColor(232, 100, 23));

    velGraph->series[2]->setName("Avg 5");
    velGraph->series[2]->setColor(QColor(111, 222, 141));

    velGraph->chart->setTitle("Velocidad vs Tiempo");
    velGraph->axisX->setTitleText("Tiempo");
    velGraph->axisY->setTitleText("Velocidad");
    velGraph->axisY->setRange(0, 480); // Limits in y axis

    // Fill with 0
    for (int i = 0; i < 99; ++i) // One less than position
        velGraph->series[0]->append(0,0);
    for (int i = 0; i < 98; ++i)  // 98 because the method centered differences misses two elements
        velGraph->series[1]->append(0,0);
    for (int i = 0; i < 96; ++i) // using 5 data to average, misses 4
        velGraph->series[2]->append(0,0);

    layout->addWidget(velGraph->chartView, 2, 0, 1, 2);



    // Acceleration Chart
    accGraph->series[0]->setName("Directa");
    accGraph->series[0]->setColor(QColor(255, 187, 28));

    accGraph->series[1]->setName("Centrada");
    accGraph->series[1]->setColor(QColor(232, 100, 23));

    accGraph->chart->setTitle("Aceleración vs Tiempo");
    accGraph->axisX->setTitleText("Tiempo");
    accGraph->axisY->setTitleText("Aceleración");
    accGraph->axisY->setRange(-240, 240); // Limits in y axis

    for (int i = 0; i < 97; ++i) { // Fill with 0, 97 because the method centered differences
        accGraph->series[0]->append(0,0);
        accGraph->series[1]->append(0,0);
    }
    accGraph->series[0]->append(0,0); // 98 data for direct method

    layout->addWidget(accGraph->chartView, 2, 2, 1, 2);



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
