#ifndef ARDUINO_H
#define ARDUINO_H

#define ESP_PLATFORM // Uncomment to use with ESP32

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>

class Arduino : public QObject
{
    Q_OBJECT

public:
    Arduino(QObject* parent);
    ~Arduino();

    long position = 0;

    void write(const QByteArray data);

private slots:
    void getEncoderData();

private:
    QSerialPort serial;


};

#endif // ARDUINO_H
