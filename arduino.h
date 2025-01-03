#ifndef ARDUINO_H
#define ARDUINO_H

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

    void open(const QString& serialPortName);

    void write(const QByteArray data);

private slots:
    void getEncoderData();

private:
    QSerialPort serial;


};

#endif // ARDUINO_H
