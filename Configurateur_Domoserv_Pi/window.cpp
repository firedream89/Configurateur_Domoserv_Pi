#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);

    ui->statusBar->showMessage("Attente connexion...");

    connect(ui->bUpdate,SIGNAL(clicked(bool)),this,SLOT(UpdateData()));
    connect(ui->bDelProg,SIGNAL(clicked(bool)),this,SLOT(DelProg()));
    connect(ui->bAddProg,SIGNAL(clicked(bool)),this,SLOT(AddProg()));
    connect(ui->bAddIp,SIGNAL(clicked(bool)),this,SLOT(AddIp()));
    connect(ui->bRemoveIp,SIGNAL(clicked(bool)),this,SLOT(DelIp()));
    connect(ui->bReload,SIGNAL(clicked(bool)),this,SLOT(Reload()));
    connect(ui->actionAbout_Qt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionQuitter,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->bReloadLog,SIGNAL(clicked(bool)),this,SLOT(ReloadLog()));

    ConnectDialog();
}

Window::~Window()
{
    delete ui;
}

void Window::ConnectDialog()
{
    QDialog *d = new QDialog(this);
    d->setObjectName("connexion");
    connect(d,&QDialog::accepted,this,&Window::GetConnection);
    ui2 = new Ui::Connexion;
    ui2->setupUi(d);
    d->exec();
}

void Window::GetConnection()
{
    QDialog *d = this->findChild<QDialog*>("connexion");
    if(d) {
        ip = d->findChild<QLineEdit*>("eIp")->text();
        port = d->findChild<QSpinBox*>("ePort")->value();
        password = d->findChild<QLineEdit*>("ePassword")->text();
        keySize = d->findChild<QSpinBox*>("eKeySize")->value();
        codeSize = d->findChild<QSpinBox*>("eCodeSize")->value();
        charset = d->findChild<QComboBox*>("eCharset")->currentIndex();

        ConnectToHost();
    }
}

void Window::GetData()
{
    ui->statusBar->showMessage("Updating data...");

    Send_Data("Config|General;GETCVOrder");
    Send_Data("Config|CVOrder;GETProg");
    Send_Data("Config|CVOrder;GETConfig");
    Send_Data("Config|CVOrder;GPIO");
    Send_Data("Config|Server;GETPort");
    Send_Data("Config|Server;GETPassword");
    Send_Data("Config|Server;GETUserSocket");
    Send_Data("Config|Server;GETAdminSocket");
    Send_Data("Config|Server;GETWebPort");
    Send_Data("Config|Server;GETWebPassword");
    Send_Data("Config|Server;GetAdminCrypto");
    Send_Data("Config|Server;GetUserCrypto");
}

void Window::Ready()
{
    ui->tabWidget->setVisible(true);
    ui->tab->setEnabled(true);

    if(_dataResult.contains("GETLog"))
    {
        ReloadLog();
        return;
    }
    else if(_dataResult.contains("General;CVOrder"))//0
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
    }
    else if(_dataResult.contains("GETProg"))//1
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
    }
    else if(_dataResult.contains("GETConfig"))//2
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
                else if(result.at(i).contains("ActCPTEnergy"))
                {
                    ui->SetActCPTEnergy->setChecked(result.at(i).split("=").last().toInt());
                    if(result.at(i).split("=").last().toInt() == 0)
                        ui->ActCPTEnergy->setText("Non");
                    else
                        ui->ActCPTEnergy->setText("Oui");
                }
                else if(result.at(i).contains("ActHCCPTEnergy"))
                {
                    ui->SetActHCCPTEnergy->setChecked(result.at(i).split("=").last().toInt());
                    if(result.at(i).split("=").last().toInt() == 0)
                        ui->ActHCCPTEnergy->setText("Non");
                    else
                        ui->ActHCCPTEnergy->setText("Oui");
                }
                else if(result.at(i).contains("ImpWattCPTEnergy"))
                {
                    ui->WattCPTEnergy->setText(result.at(i).split("=").last());
                    ui->SetWattCPTEnergy->setValue(result.at(i).split("=").last().toInt());
                }

            }
        }
        _dataResult.clear();
    }
    else if(_dataResult.contains("GPIO"))//3
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
                else if(result.at(i).contains("ImpCPTEnergy"))
                    ui->reverseOnOff->setChecked(result.at(i).split("=").last().toInt());
                else if(result.at(i).contains("HCCPTEnergy"))
                    ui->reverseOnOff->setChecked(result.at(i).split("=").last().toInt());
            }
        }
        _dataResult.clear();
    }
    else if(_dataResult.contains("GETPort"))//4
    {
        if(_dataResult.split("=").count() == 2)
            ui->ePortServer->setValue(_dataResult.split("=").last().toInt());
        _dataResult.clear();
    }
    else if(_dataResult.contains("GetPassword"))//5
    {
        if(_dataResult.split("=").count() == 2)
            ui->ePassword_2->setText(_dataResult.split("=").last());
        _dataResult.clear();
    }
    else if(_dataResult.contains("GETUserSocket"))//6
    {
        if(_dataResult.split("=").count() == 2)
            ui->cWebSocket->setChecked(_dataResult.split("=").last().toInt());
        _dataResult.clear();
    }
    else if(_dataResult.contains("GETAdminSocket"))//7
    {
        if(_dataResult.split("=").count() == 2)
            ui->cAdminSocket->setChecked(_dataResult.split("=").last().toInt());
        _dataResult.clear();
    }
    else if(_dataResult.contains("GETWebPort"))//8
    {
        if(_dataResult.split("=").count() == 2)
            ui->eWebPort->setValue(_dataResult.split("=").last().toInt());
        _dataResult.clear();
    }
    else if(_dataResult.contains("GetWebPassword"))//9
    {
        if(_dataResult.split("=").count() == 2)
            ui->eWebPassword->setText(_dataResult.split("=").last());
        _dataResult.clear();
    }
    else if(_dataResult.contains("GetAdminCrypto")) {
        if(_dataResult.split("=").count() == 2 && _dataResult.split("=").last().split(";").count() == 3) {
            QStringList list = _dataResult.split("=").last().split(";");
            ui->eAKeySize->setValue(list.at(0).toInt());
            ui->eACodeSize->setValue(list.at(1).toInt());
            ui->eACharFormat->setCurrentIndex(list.at(2).toInt());
        }
    }
    else if(_dataResult.contains("GetUserCrypto")) {
        if(_dataResult.split("=").count() == 2 && _dataResult.split("=").last().split(";").count() == 3) {
            QStringList list = _dataResult.split("=").last().split(";");
            ui->eUKeySize->setValue(list.at(0).toInt());
            ui->eUCodeSize->setValue(list.at(1).toInt());
            ui->eUCharFormat->setCurrentIndex(list.at(2).toInt());

            ui->statusBar->showMessage("Done");
        }
    }
}

void Window::UpdateData(bool reset)
{
    int process(0);

    if(process == 0)
    {
        int result(0);
        if(ui->cCVOrder->isChecked())
            result = 1;

        Send_Data("Config|General;SETCVOrder=" + QString::number(result));
        process++;
    }
    if(process == 1)
    {

        Send_Data("Config|Server;SETPassword=" + ui->ePassword_2->text());
        process++;
    }
    if(process == 2)
    {

        Send_Data("Config|Server;SETPort=" + QString::number(ui->ePortServer->value()));
        process++;
    }
    if(process == 3)
    {

        int result(0);
        if(ui->cWebSocket->isChecked())
            result = 1;
        Send_Data("Config|Server;SETUserSocket=" + QString::number(result));
        process++;
    }
    if(process == 4)
    {

        int result(0);
        if(ui->cAdminSocket->isChecked())
            result = 1;
        Send_Data("Config|Server;SETAdminSocket=" + QString::number(result));
        process++;
    }
    if(process == 5)
    {
        Send_Data("Config|CVOrder;SETConfig;Priority=" + QString::number(ui->cPriority->currentIndex()));
        process++;
    }
    if(process == 6)
    {
        Send_Data("Config|CVOrder;SETConfig;timerNetwork=" + QString::number(ui->eTIp->value()));
        process++;
    }
    if(process == 7)
    {
        QString result("Config|CVOrder;SETGPIO");
        result += ";Z1Eco=" + QString::number(ui->gpioZ1Eco->value());
        result += ";Z1HG=" + QString::number(ui->gpioZ1HG->value());
        result += ";Z2Eco=" + QString::number(ui->gpioZ2Eco->value());
        result += ";Z2HG=" + QString::number(ui->gpioZ2HG->value());
        result += ";ImpCPTEnergy=" + QString::number(ui->gpioImpCPT->value());
        result += ";HCCPTEnergy=" + QString::number(ui->gpioHCCPT->value());

        Send_Data(result);
        process++;
    }
    if(process == 8)
    {
        Send_Data("Config|Server;SETWebPort=" + QString::number(ui->eWebPort->value()));
        process++;
    }
    if(process == 9)
    {
        Send_Data("Config|Server;SETWebPassword=" + ui->eWebPassword->text());
        process++;
    }
    if(process == 10)
    {
        QString checked = "0";
        if(ui->SetActCPTEnergy->isChecked())
            checked = "1";
        Send_Data("Config|CVOrder;SETConfig;ActCPTEnergy=" + checked);
        process++;
    }
    if(process == 11)
    {
        QString checked = "0";
        if(ui->SetActHCCPTEnergy->isChecked())
            checked = "1";
        Send_Data("Config|CVOrder;SETConfig;ActHCCPTEnergy=" + checked);
        process++;
    }
    if(process == 12)
    {
        Send_Data("Config|CVOrder;SETConfig;ImpWattCPTEnergy=" + QString::number(ui->SetWattCPTEnergy->value()));
        process++;
    }
    if(process == 13)
    {
        Send_Data(QString("Config|Server;SetAdminCrypto=%0;%1;%2").arg(ui->eAKeySize->value()).arg(ui->eACodeSize->value()).arg(ui->eACharFormat->currentIndex()));
        process++;
    }
    if(process == 14)
    {
        Send_Data(QString("Config|Server;SetUserCrypto=%0;%1;%2").arg(ui->eUKeySize->value()).arg(ui->eUCodeSize->value()).arg(ui->eUCharFormat->currentIndex()));
        process++;
    }
    if(process == 15 || reset)
    {
        process = 0;
    }
}

void Window::Reload()
{
    Send_Data("Reload|");
}

void Window::ReloadLog()
{
    if(qobject_cast<QPushButton *>(sender()) == nullptr)
    {
        ui->bReloadLog->setEnabled(true);
        ui->log->clear();
        _dataResult.replace("Config|General;GETLog=","");
        ui->log->setText(_dataResult);
        _dataResult.clear();
    }
    else
    {
        ui->bReloadLog->setEnabled(false);
        Send_Data("Config|General;GETLog");
    }
}

void Window::DelProg()
{
    Send_Data("Config|CVOrder;DELProg");
    Ready();
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
    Ready();
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
            Ready();
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
            Ready();
        }
    }
}

void Window::ConnectToHost()
{
    c = new client(client::Web,client::Admin,"ConfigDomoservApp");
    connect(c,&client::Ready,this,&Window::GetData);
    connect(c,&client::ReceiptData,this,&Window::Receipt_Data);

    ui->statusBar->showMessage("Connexion au serveur...");
    c->SetCrypto(keySize,codeSize,charset);
    c->Connect(ip,port,password);
}

void Window::Send_Data(QString data)
{
    c->SendToServer(data);
}

void Window::Receipt_Data(QString data)
{
    _dataResult = data;
    Ready();
}
