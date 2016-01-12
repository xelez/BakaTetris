#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkInterface>

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    udpSocket->bind(QHostAddress::AnyIPv4, 13333, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    QHostAddress group = QHostAddress("224.0.0.111");
    QList<QNetworkInterface> mListIfaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i < mListIfaces.length(); i++)
    {
        i = 4;
        qDebug() << mListIfaces.at(i).humanReadableName();
        qDebug() << udpSocket->joinMulticastGroup(group, mListIfaces.at(i));
        break;
    }

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
    //udpSocket->joinMulticastGroup(group);
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
