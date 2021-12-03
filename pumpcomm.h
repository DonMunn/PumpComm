#ifndef PUMPCOMM_H
#define PUMPCOMM_H

#include <QObject>
#include <QSerialPort>
#include <QQueue>
#include <QTimer>
#include <QDebug>
#include "settingsdialog.h"


class PumpComm : public QObject
{
    Q_OBJECT
public:
    enum commands {NONE, SETTUBEDIAMETER, SETDISPENSEMODE, GETDISPENSERATE, SETDISPENSERATE, SETDISPENSEVOLUME, SETDISPENSE};

    PumpComm();

    void setTubeDia(double diameter); // Done
    void setDispMode(); // Done
    void getCalDispRate(); // Done
    void setDispRate(double disp_rate_p); // Done
    void setDispVol(double volume); // Done

    void setDispense(); // Done

    void openSerialPort(); // Done
    void closeSerialPort(); // Done

    bool isOpen(); // Done

    void updateSerialInfo(const SettingsDialog::Settings &settings); // Done

private:
    void sendError(QSerialPort::SerialPortError error, const QString &error_message); // Done
    QString getCommand(commands command); // Done
    void serialConnSendMessage(); // Done
    QSerialPort *serial_conn = new QSerialPort(this);

    QString temp_data = "";
    QQueue<commands> command_queue;
    QQueue<int> data_queue;

    QTimer *timer = new QTimer(this);


signals:
    void rawDataSignal(QString data);
    void returnData(QString data, commands command_sent);
    void errorSignal(QSerialPort::SerialPortError error, QString error_string, commands command_sent);

private slots:
    void collectErrorData(QSerialPort::SerialPortError error);
    void serialConnReceiveMessage();
    void timeout();
};

#endif // PUMPCOMM_H
