#include "pumpcomm.h"

PumpComm::PumpComm(QObject *parent) : SerialComm(parent) {
    connect(&serial_conn, &QSerialPort::readyRead, this, &PumpComm::serialConnReceiveMessage);
    connect(&send_message_timer, &QTimer::timeout, this, &PumpComm::sendMessage);
}

void PumpComm::setTubeDia(double diameter) {
    if (isOpen()) {
        command_queue.enqueue(SETTUBEDIAMETER);
        data_queue.enqueue(QString::number((int)(diameter*100.0)));
    } else {
        sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::setDispMode() {
    if (isOpen()) {
        command_queue.enqueue(SETDISPENSEMODE);
    } else {
        sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::getCalDispRate() {
    if (isOpen()) {
        command_queue.enqueue(GETDISPENSERATE);
    } else {
        sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::setDispRate(double disp_rate_p) {
    if (isOpen()) {
        command_queue.enqueue(SETDISPENSERATE);
        data_queue.enqueue(QString::number((int)(disp_rate_p * 10.0)));
    } else {
       sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::setDispVol(double volume) {
    if (isOpen()) {
        command_queue.enqueue(SETDISPENSEVOLUME);
        data_queue.enqueue(QString::number((int)(volume * 100.0)));
    } else {
       sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::setDispense() {
    if (isOpen()) {
        command_queue.enqueue(SETDISPENSE);
    } else {
        sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::setRotationClockwise() {
    if (isOpen()) {
        command_queue.enqueue(SETROTATIONCLOCKWISE);
    } else {
        sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

void PumpComm::setRotationCounterClockwise() {
    if (isOpen()) {
        command_queue.enqueue(SETROTATIONCOUNTERCLOCKWISE);
    } else {
        sendError(QSerialPort::NotOpenError, "No open connection");
    }
}

//Private
void PumpComm::serialConnSendMessage() {
    commands command = (commands)command_queue.head();
    QByteArray data = getCommand(command).toUtf8();

    if (command == SETTUBEDIAMETER) {
        data += data_queue.head().rightJustified(4, '0').toUtf8();
    } else if (command == SETDISPENSEVOLUME | command == SETDISPENSERATE) {
        data += data_queue.head().rightJustified(5, '0').toUtf8();
    }

    data += '\r';

    qDebug() << "final data:" << data;

    if (serial_conn.write(data) == -1) { // -1 indicates error occurred
        // send QSerialPort::NotOpenError if QOIDevice::NotOpen is triggered
        if (serial_conn.error() == QSerialPort::NoError) {
            sendError(QSerialPort::NotOpenError, "No open connection");
        } //else UNNEEDED as the QSerialPort will emit its own signal for other errors
    } else {
        emit rawDataSignal(data);
        timeout_timer.start(1000);
    }
}

QString PumpComm::getCommand(commands command) {
    switch (command) {
        case NONE:
            return "";
        case SETTUBEDIAMETER:
            return "1+";
        case SETDISPENSEMODE:
            return "1O";
        case GETDISPENSERATE:
            return "1!";
        case SETDISPENSERATE:
            return "1S";
        case SETDISPENSEVOLUME:
            return "1[";
        case SETDISPENSE:
            return "1H";
    }
}

void PumpComm::sendError(QSerialPort::SerialPortError error, const QString &error_message) {
    send_message_timer.stop();

    // clear serial internal read/write buffers
    if (isOpen()) {
        serial_conn.clear();
    }

    // Dequeue command if one is associated with the error
    if (command_queue.isEmpty()) {
        emit errorSignal(error, error_message, commands::NONE);
    } else {
        commands command = (commands)command_queue.dequeue();
        if (command == SETTUBEDIAMETER || command == SETDISPENSERATE || command == SETDISPENSEVOLUME)
            data_queue.dequeue();

        temp_data = "";
        emit errorSignal(error, error_message, command);
    }
}

//Slots
void PumpComm::serialConnReceiveMessage() {
    // construct message from parts
    temp_data += serial_conn.readAll();

    QRegExp re = QRegExp("^\\s*(\\d*\\.\\d*) ml\\/min\\s{2}$");
    if(QRegExp("^\\*$").exactMatch(temp_data) || re.exactMatch(temp_data)) {
        timeout_timer.stop();
        commands command = (commands)command_queue.dequeue();

        if (command == SETTUBEDIAMETER || command == SETDISPENSERATE || command == SETDISPENSEVOLUME)
            data_queue.dequeue();

        // Extract value from return
        if (command == GETDISPENSERATE) temp_data = re.cap(1);
        emit returnData(temp_data, command);

        temp_data = "";
    } else if (QRegExp("^#$").exactMatch(temp_data)) {
        sendError(QSerialPort::UnsupportedOperationError, "Incorrect command sent");
    }
}

void PumpComm::sendMessage() {
    if (isOpen() && !command_queue.isEmpty() && !timeout_timer.isActive()) {
        serialConnSendMessage();
    }
}
