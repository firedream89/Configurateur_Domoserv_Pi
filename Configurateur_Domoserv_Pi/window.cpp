#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);

    dataSize = 0;

    ui->tabWidget->setVisible(false);
    this->resize(this->width(),ui->formConnect->height());

    ui->statusBar->showMessage("Attente connexion...");

    connect(ui->bConnect,SIGNAL(clicked(bool)),this,SLOT(ConnectToHost()));
    connect(ui->bUpdate,SIGNAL(clicked(bool)),this,SLOT(UpdateData()));
    connect(ui->bDelProg,SIGNAL(clicked(bool)),this,SLOT(DelProg()));
    connect(ui->bAddProg,SIGNAL(clicked(bool)),this,SLOT(AddProg()));
    connect(ui->bAddIp,SIGNAL(clicked(bool)),this,SLOT(AddIp()));
    connect(ui->bRemoveIp,SIGNAL(clicked(bool)),this,SLOT(DelIp()));
    connect(ui->bReload,SIGNAL(clicked(bool)),this,SLOT(Reload()));
    connect(ui->actionAbout_Qt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionQuitter,SIGNAL(triggered()),this,SLOT(close()));
}

Window::~Window()
{
    delete ui;
}

void Window::Ready(bool reset)
{
    if(ui->formConnect->isVisible())
    {
        this->resize(this->width(),ui->tabWidget->height());
    }

    ui->formConnect->setVisible(false);
    ui->tabWidget->setVisible(true);
    ui->tab->setEnabled(true);

    static int process(0);

    if(reset)
        process = 0;

    if(process == 0)
    {
        if(_dataResult.isEmpty())
        {
            ui->statusBar->showMessage("Récupération etat gestionnaire chauffage depuis le serveur...");
            Send_Data("Config|General;GETCVOrder");
        }
        else
        {
            if(_dataResult.split("=").last().toInt() == 1)
            {
                ui->lInfoGeneral->setText("Gestionnaire chauffage : Actif\n");
                ui->cCVOrder->setChecked(true);
                ui->tab_2->setEnabled(true);
            }
            else
            {
                ui->lInfoGeneral->setText("Gestionnaire chauffage : Inactif\n");
                ui->cCVOrder->setChecked(false);
                ui->tab_2->setEnabled(false);
            }
            _dataResult.clear();
            process++;

        }
    }
    if(ui->tab_2->isEnabled())
    {
        if(process == 1)
        {
            if(_dataResult.isEmpty())
            {
                ui->statusBar->showMessage("Récupération programmation chauffage depuis le serveur...");
                Send_Data("Config|CVOrder;GETProg");
            }
            else
            {

                ui->lInfoCVOrder->setText("Programmation :\n");
                QString zone1,zone2;
                for(int i=0;i<_dataResult.split(";").count();i++)
                {
                    QStringList date = _dataResult.split(";").at(i).split("#");
                    if(date.count() == 3)
                    {
                        QString state;
                        if(date.at(2).toInt() == confort)
                            state = "Confort";
                        else if(date.at(2).toInt() == eco)
                            state = "Eco";
                        else if(date.at(2).toInt() == horsGel)
                            state = "Hors Gel";
                        else
                            state = "Erreur";

                        QString zone;
                        if(date.at(1).toInt() == 0)
                            zone = "Zone 1";
                        else if(date.at(1).toInt() == 1)
                            zone = "Zone 2";
                        else
                            zone = "Erreur";

                        if(date.at(1).toInt() == 0)
                            zone1 += date.at(0) + " " + state + "\n";
                        else if(date.at(1).toInt() == 1)
                            zone2 += date.at(0) + " " + state + "\n";
                    }
                }
                ui->lInfoCVOrder->setText(ui->lInfoCVOrder->text() + "Zone 1 :\n" +  zone1 + "\nZone 2 :\n" + zone2 + "\n");
                _dataResult.clear();
                process++;
            }
        }
        if(process == 2)
        {
            if(_dataResult.isEmpty())
            {
                ui->statusBar->showMessage("Récupération config gestionnaire chauffage depuis le serveur...");
                Send_Data("Config|CVOrder;GETConfig");
            }
            else
            {
                QStringList result = _dataResult.split(";");
                ui->lInfoCVOrder->setText(ui->lInfoCVOrder->text() + "\nIp :\n");
                for(int i=0;i<result.count();i++)
                {
                    if(result.at(i).split("=").count() == 2)
                    {
                        if(result.at(i).contains("Priority"))
                            ui->cPriority->setCurrentIndex(result.at(i).split("=").last().toInt());
                        else if(result.at(i).contains("NetworkTimer"))
                            ui->eTIp->setValue(result.at(i).split("=").last().toInt() / 1000);
                        else if(result.at(i).contains("IpPing"))
                            ui->lInfoCVOrder->setText(ui->lInfoCVOrder->text() + result.at(i).split("=").last() + "\n");
                    }
                }
                _dataResult.clear();
                process++;
            }
        }
        if(process == 3)
        {
            if(_dataResult.isEmpty())
            {
                ui->statusBar->showMessage("Récupération GPIO Chauffage depuis le serveur...");
                Send_Data("Config|CVOrder;GPIO");
            }
            else
            {
                QStringList result = _dataResult.split(";");

                for(int i=0;i<result.count();i++)
                {
                    if(result.at(i).contains("="))
                    {
                        if(result.at(i).contains("Z1Eco"))
                            ui->gpioZ1Eco->setValue(result.at(i).split("=").last().toInt());
                        else if(result.at(i).contains("Z1HG"))
                            ui->gpioZ1HG->setValue(result.at(i).split("=").last().toInt());
                        else if(result.at(i).contains("Z2Eco"))
                            ui->gpioZ2Eco->setValue(result.at(i).split("=").last().toInt());
                        else if(result.at(i).contains("Z2HG"))
                            ui->gpioZ2HG->setValue(result.at(i).split("=").last().toInt());
                        else if(result.at(i).contains("ReverseOnOff"))
                            ui->reverseOnOff->setChecked(result.at(i).split("=").last().toInt());
                    }
                }
                _dataResult.clear();
                process++;
            }
        }
    }
    else
        process = 4;

    if(process == 4)
    {
        if(_dataResult.isEmpty())
        {
            ui->statusBar->showMessage("Récupération Port serveur depuis le serveur...");
            Send_Data("Config|Server;GETPort");
        }
        else
        {
            if(_dataResult.split("=").count() == 2)
                ui->ePortServer->setValue(_dataResult.split("=").last().toInt());
            _dataResult.clear();
            process++;
        }
    }
    if(process == 5)
    {
        if(_dataResult.isEmpty())
        {
            ui->statusBar->showMessage("Récupération Password serveur depuis le serveur...");
            Send_Data("Config|Server;GETPassword");
        }
        else
        {
            if(_dataResult.split("=").count() == 2)
                ui->ePassword_2->setText(_dataResult.split("=").last());
            _dataResult.clear();
            process++;
        }
    }
    if(process == 6)
    {
        if(_dataResult.isEmpty())
        {
            ui->statusBar->showMessage("Récupération WebSocket depuis le serveur...");
            Send_Data("Config|Server;GETWebSocket");
        }
        else
        {
            if(_dataResult.split("=").count() == 2)
                ui->cWebSocket->setChecked(_dataResult.split("=").last().toInt());
            _dataResult.clear();
            process++;
        }
    }
    if(process == 7)
    {
        if(_dataResult.isEmpty())
        {
            ui->statusBar->showMessage("Récupération WebPort depuis le serveur...");
            Send_Data("Config|Server;GETWebPort");
        }
        else
        {
            if(_dataResult.split("=").count() == 2)
                ui->eWebPort->setValue(_dataResult.split("=").last().toInt());
            _dataResult.clear();
            process++;
        }
    }
    if(process == 8)
    {
        if(_dataResult.isEmpty())
        {
            ui->statusBar->showMessage("Récupération Web Password depuis le serveur...");
            Send_Data("Config|Server;GETWebPassword");
        }
        else
        {
            if(_dataResult.split("=").count() == 2)
                ui->eWebPassword->setText(_dataResult.split("=").last());
            _dataResult.clear();
            process++;
        }
    }
}

void Window::UpdateData(bool reset)
{
    int process(0);

    if(reset)
        process = 0;

    if(process == 0)
    {
        int result(0);
        if(ui->cCVOrder->isChecked())
            result = 1;

        Send_Data("Config|General;SETCVOrder=" + QString::number(result));
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 1)
    {

        Send_Data("Config|Server;SETPassword=" + ui->ePassword_2->text());
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 2)
    {

        Send_Data("Config|Server;SETPort=" + QString::number(ui->ePortServer->value()));
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 3)
    {

        int result(0);
        if(ui->cWebSocket->isChecked())
            result = 1;
        Send_Data("Config|Server;SETWebSocket=" + QString::number(result));
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 4)
    {
        Send_Data("Config|CVOrder;SETConfig;Priority=" + QString::number(ui->cPriority->currentIndex()));
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 5)
    {
        Send_Data("Config|CVOrder;SETConfig;timerNetwork=" + QString::number(ui->eTIp->value()));
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 6)
    {
        QString result("Config|CVOrder;SETGPIO");
        result += ";Z1Eco=" + QString::number(ui->gpioZ1Eco->value());
        result += ";Z1HG=" + QString::number(ui->gpioZ1HG->value());
        result += ";Z2Eco=" + QString::number(ui->gpioZ2Eco->value());
        result += ";Z2HG=" + QString::number(ui->gpioZ2HG->value());

        Send_Data(result);
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 6)
    {
        Send_Data("Config|Server;SETWebPort=" + QString::number(ui->eWebPort->value()));
        process++;
    }
    socket->waitForBytesWritten();
    if(process == 7)
    {
        Send_Data("Config|Server;SETWebPassword=" + ui->eWebPassword->text());
        process++;
    }
    Ready(true);
}

void Window::Reload()
{
    Send_Data("Reload|");
}

void Window::DelProg()
{
    Send_Data("Config|CVOrder;DELProg");
    Ready(true);
}

void Window::AddProg()
{
    QString zone;
    if(ui->rZ1->isChecked())
        zone = "0";
    else
        zone = "1";

    QString state;
    if(ui->rConfort->isChecked())
        state = "0";
    else
        state = "1";

    QString result("Config|CVOrder;SETProg");
    QStringList day;
    if(ui->cMonday->isChecked())
        day.append("01");
    if(ui->cTuesday->isChecked())
        day.append("02");
    if(ui->cWednesday->isChecked())
        day.append("03");
    if(ui->cThurday->isChecked())
        day.append("04");
    if(ui->cFriday->isChecked())
        day.append("05");
    if(ui->cSaturday->isChecked())
        day.append("06");
    if(ui->cSunday->isChecked())
        day.append("07");

    for(int i=0;i<day.count();i++)
        result += ";2018-01-" + day.at(i) + " " + ui->hour->text() + "#" + zone + "#" + state;

    Send_Data(result);
    Ready(true);
}

void Window::AddIp()
{
    if(ui->eAddIp->text().split(".").count() == 4)
    {
        if(ui->eAddIp->text().split(".").at(0).toInt() > 0 && ui->eAddIp->text().split(".").at(0).toInt() < 255 &&
                ui->eAddIp->text().split(".").at(1).toInt() > 0 && ui->eAddIp->text().split(".").at(1).toInt() < 255 &&
                ui->eAddIp->text().split(".").at(2).toInt() > 0 && ui->eAddIp->text().split(".").at(2).toInt() < 255 &&
                ui->eAddIp->text().split(".").at(3).toInt() > 0 && ui->eAddIp->text().split(".").at(3).toInt() < 255)
        {
            Send_Data("Config|CVOrder;SETConfig;AddIpPing=" + ui->eAddIp->text());
            Ready(true);
        }
    }
}

void Window::DelIp()
{
    if(ui->eAddIp->text().split(".").count() == 4)
    {
        if(ui->eAddIp->text().split(".").at(0).toInt() > 0 && ui->eAddIp->text().split(".").at(0).toInt() < 255 &&
                ui->eAddIp->text().split(".").at(1).toInt() > 0 && ui->eAddIp->text().split(".").at(1).toInt() < 255 &&
                ui->eAddIp->text().split(".").at(2).toInt() > 0 && ui->eAddIp->text().split(".").at(2).toInt() < 255 &&
                ui->eAddIp->text().split(".").at(3).toInt() > 0 && ui->eAddIp->text().split(".").at(3).toInt() < 255)
        {
            Send_Data("Config|CVOrder;SETConfig;RmIpPing=" + ui->eAddIp->text());
            Ready(true);
        }
    }
}

void Window::ConnectToHost()
{
    ui->bConnect->setEnabled(false);
    ui->statusBar->showMessage("Connexion au serveur...");
    socket = new QTcpSocket;
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorServer(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(connected()),this,SLOT(Send_Data()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(Receipt_Data()));
    socket->connectToHost(ui->eIp->text(),ui->ePort->text().toInt());
}

void Window::errorServer(QAbstractSocket::SocketError err)
{
    ui->statusBar->showMessage("Erreur de connexion au serveur : " + socket->errorString(),15000);
    ui->bConnect->setEnabled(true);
}

void Window::Send_Data(QString data)
{
    qDebug() << data;
    if(data.isEmpty())
        data = ui->ePassword->text();
    QByteArray paquet;

    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0;
    out << Encrypt(data);
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));


    if(PKEY.isEmpty())
        ui->statusBar->showMessage("Authentification...");

    socket->write(paquet);
}

void Window::Receipt_Data()
{
    if (socket == 0)
        return;

    QDataStream in(socket);

    if(dataSize == 0)
    {
        if(socket->bytesAvailable() < (int)sizeof(quint16))
             return;

        in >> dataSize;
    }

    if(socket->bytesAvailable() < dataSize)
        return;

    QString data;
    in >> data;

    if(PKEY.isEmpty())
    {
        PKEY = data;
        ui->statusBar->showMessage("Connecté au serveur",15000);
        Ready();
    }
    else
    {
        ui->statusBar->showMessage("Terminé",5000);
        _dataResult = Decrypt(data);
        Ready();
    }

    dataSize = 0;
}

QString Window::Encrypt(QString text)
{
    QString crypt;
    QStringList k = PKEY.split(" ");
    int idk(0);
    for(int i = 0;i<text.count();i++)
    {
        if(idk == k.count())
        {
            idk = 0;
        }
        int t = text.at(i).unicode();
        t -= k.at(idk).toInt();
        if(t > 250)
        {
            t = t - 250;
        }
        else if(t < 0)
        {
            t = t + 250;
        }
        if(t == 34)//si '
        {
            t = 251;
        }
        else if(t == 39)//ou "
        {
            t = 252;
        }
        crypt += QChar(t).toLatin1();
        idk++;
    }
    return crypt;
}

QString Window::Decrypt(QString text)
{
    QString decrypt;
    QStringList k = PKEY.split(" ");
    int idk(0);
    for(int i = 0;i<text.count();i++)
    {
        if(idk == k.count())
        {
            idk = 0;
        }
        int t = text.at(i).unicode();
        if(t == 251)//retour a '
        {
            t = 34;
        }
        else if(t == 252)//retour a "
        {
            t = 39;
        }
        t += k.at(idk).toInt();
        if(t < 0)
        {
            t = t + 250;
        }
        else if(t > 250)
        {
            t = t - 250;
        }
        decrypt += QChar(t).toLatin1();
        idk++;
    }
    return decrypt;
}
