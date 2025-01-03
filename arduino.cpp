#include "arduino.h"


Arduino::Arduino(QObject* parent)
    : QObject(parent)
{
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
}

Arduino::~Arduino()
{
}

void Arduino::open(const QString& serialPortName)
{
    serial.setPortName(serialPortName);

    if (!serial.open(QIODevice::ReadWrite)) {
        throw std::runtime_error(serial.errorString().toStdString());
    }

    connect(&serial, &QSerialPort::readyRead, this, &Arduino::getEncoderData);
}

void Arduino::write(const QByteArray data)
{
    serial.write(data);
}


void Arduino::getEncoderData()
{
    QByteArray data = serial.readAll();

    for (char d : data){
        if (d == '1') {
            position += 12;
        }
        if (d == '0') {
            position -= 12;
        }
    }

}
