#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

enum order{
    confort,
    eco,
    horsGel
};

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();
    QString Decrypt(QString text);
    QString Encrypt(QString text);

public slots:
    void Send_Data(QString data = nullptr);
    void Receipt_Data();
    void Ready(bool reset = 0);
    void errorServer(QAbstractSocket::SocketError err);
    void ConnectToHost();
    void UpdateData(bool reset = 0);
    void AddProg();
    void AddIp();
    void DelProg();
    void DelIp();
    void Reload();

private:
    QTcpSocket *socket;
    quint16 dataSize;
    QString _dataResult;
    QString PKEY = "";
    QString password;

private:
    Ui::Window *ui;
};

#endif // WINDOW_H
