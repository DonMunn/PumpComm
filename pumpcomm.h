#ifndef PUMPCOMM_H
#define PUMPCOMM_H

#include "serialcomm.h"
#include "settingsdialog.h"


class PumpComm : public SerialComm
{
    Q_OBJECT
public:
    enum commands {NONE, SETTUBEDIAMETER, SETDISPENSEMODE, GETDISPENSERATE, SETDISPENSERATE, SETDISPENSEVOLUME, SETDISPENSE};

    explicit PumpComm(QObject *parent = nullptr);

    void setTubeDia(double diameter); // Done
    void setDispMode(); // Done
    void getCalDispRate(); // Done
    void setDispRate(double disp_rate_p); // Done
    void setDispVol(double volume); // Done

    void setDispense(); // Done

protected:
    void sendError(QSerialPort::SerialPortError error, const QString &error_message) override; // Done
    void serialConnSendMessage() override; // Done

private:
    QString getCommand(commands command); // Done

private slots:
    void serialConnReceiveMessage() override;

};

#endif // PUMPCOMM_H
