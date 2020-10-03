#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QObject>
#include "../../../ServerFire/Client/src/client.h"
#include <QDialog>
#include "ui_connexion.h"

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

public slots:
    void Send_Data(QString data);
    void Receipt_Data(QString data);
    void Ready();
    void ConnectToHost();
    void UpdateData(bool reset = 0);
    void AddProg();
    void AddIp();
    void DelProg();
    void DelIp();
    void Reload();
    void ReloadLog();
    void ConnectDialog();
    void GetConnection();
    void GetData();

private:
    client *c;
    QString _dataResult;
    QString password;
    Ui::Window *ui;
    Ui::Connexion *ui2;
    QString ip;
    int port;
    int keySize;
    int codeSize;
    int charset;
};

#endif // WINDOW_H
