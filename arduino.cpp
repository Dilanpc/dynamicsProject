#include "arduino.h"


Arduino::Arduino(QObject* parent)
    : QObject(parent)
{

    serial.setPortName("COM3");
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        throw std::runtime_error(serial.errorString().toStdString());
    }

    connect(&serial, &QSerialPort::readyRead, this, &Arduino::getEncoderData);
}

Arduino::~Arduino()
{
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
            ++position;
        }
        if (d == '0') {
            --position;
        }
    }

}
