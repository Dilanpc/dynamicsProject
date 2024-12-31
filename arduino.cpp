#include "arduino.h"

#ifdef ESP_PLATFORM
    #define SERIALPORT "COM4"
#else
    #define SERIALPORT "COM3"
#endif


Arduino::Arduino(QObject* parent)
    : QObject(parent)
{
    serial.setPortName(SERIALPORT);
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
            position += 12;
        }
        if (d == '0') {
            position -= 12;
        }
    }

}
