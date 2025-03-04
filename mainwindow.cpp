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


void MainWindow::saveData() const
{
    static QString path = "Resultados_posicion.csv";
    QFile file(path);

    QLineSeries* serie = posGraph->series[0];


    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        stream << "Tiempo,Posición\n";

        for (int i = 0; i < serie->count(); ++i) {
            stream << serie->at(i).x() << ',' // Time
                   << serie->at(i).y() << '\n'; // Position
        }

        file.close();
    }

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
        (1000 * pos.at(99).y() - 1000 * pos.at(97).y()) / (pos.at(99).x() - pos.at(97).x())
        );


    // Deirect
    QLineSeries* direct = velGraph->series[0];

    direct->remove(0);
    direct->append(
        (pos.at(98).x() + pos.at(99).x()) / 2,
        (1000 * pos.at(99).y() - 1000 * pos.at(98).y()) / (pos.at(99).x() - pos.at(98).x())
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
    QLineSeries& velCen = *velGraph->series[1]; // Centered velocity

    QLineSeries& velAvg = *velGraph->series[2]; // Average 5 velocity


    // Deirect
    QLineSeries& direct = *accGraph->series[0];

    direct.remove(0);
    direct.append(
        (velAvg.at(95).x() + velAvg.at(94).x()) / 2, // Average time
        (1000 * velAvg.at(95).y() - 1000 * velAvg.at(94).y()) / (velAvg.at(95).x() - velAvg.at(94).x())
        );


    // Using centered differences

    QLineSeries& centered = *accGraph->series[1];

    centered.remove(0);

    centered.append(
        (velCen.at(97).x() + velCen.at(95).x()) / 2,
        (1000 * velCen.at(97).y() - 1000 * velCen.at(95).y()) / (velCen.at(97).x() - velCen.at(95).x())
        );

    // Average 4
    QLineSeries* avg = accGraph->series[2];

    QPointF point(0,0);
    for (int i = 0; i < 4; ++i) {
        point.setX( point.x() + direct.at(97-i).x() );
        point.setY( point.y() + direct.at(97-i).y() );
    }
    point.setX(point.x() / 4);
    point.setY(point.y() / 4);

    avg->remove(0);
    avg->append(point);


    // Adjust graph for centered

    accGraph->axisX->setRange(centered.at(0).x(), centered.at(95).x());

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
    accGraph = new Grapher(this, 3);

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
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveData);
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
    velGraph->series[0]->setColor(QColor(127, 93, 14));

    velGraph->series[1]->setName("Centrada");
    velGraph->series[1]->setColor(QColor(116, 25, 1));

    velGraph->series[2]->setName("Avg 5");
    velGraph->series[2]->setColor(QColor(111, 222, 141));

    velGraph->chart->setTitle("Velocidad vs Tiempo");
    velGraph->axisX->setTitleText("Tiempo");
    velGraph->axisY->setTitleText("Velocidad");
    velGraph->axisY->setRange(0, 700); // Limits in y axis

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
    accGraph->series[0]->setColor(QColor(127, 93, 14));

    accGraph->series[1]->setName("Centrada");
    accGraph->series[1]->setColor(QColor(116, 25, 1));

    accGraph->series[2]->setName("Avg 4");
    accGraph->series[2]->setColor(QColor(111, 222, 141));

    accGraph->chart->setTitle("Aceleración vs Tiempo");
    accGraph->axisX->setTitleText("Tiempo");
    accGraph->axisY->setTitleText("Aceleración");
    accGraph->axisY->setRange(-1000, 1000); // Limits in y axis

    // Fill with 0
    for (int i = 0; i < 98; ++i) // One less than speed
        accGraph->series[0]->append(0,0);
    for (int i = 0; i < 97; ++i)  // 97 because the method centered differences misses two elements
        accGraph->series[1]->append(0,0);
    for (int i = 0; i < 95; ++i) // using 5 data to average, misses 4
        accGraph->series[2]->append(0,0);
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



    // Read arduino data each 10 milliseconds
    connect(readTimer, &QTimer::timeout, this, &MainWindow::readPos);
    readTimer->start(10);




    clock->start();
}
