#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
    //udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    udpSocket->bind(QHostAddress::AnyIPv4, 13333);
    //udpSocket->joinMulticastGroup(group);

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QHostAddress group = QHostAddress("224.0.0.111");
    QByteArray datagram = "hello";
    udpSocket->writeDatagram(datagram, group, 13334);
}

void MainWindow::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress from;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &from);

        QMessageBox::information(this, "Found", tr("\"%1\" from %2").arg(datagram.data(), from.toString()));
        //statusLabel->setText(tr("Received datagram: \"%1\"")
        //                     .arg(datagram.data()));
    }
}
