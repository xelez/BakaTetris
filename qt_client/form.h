#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QDialog>
#include <QPainter>
#include <QDialog>
#include <QBasicTimer>
#include <QUdpSocket>
#include <QHostAddress>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "wsclient.h"

namespace Ui {
class Form;
}

class Form : public QDialog
{
    Q_OBJECT
private:
    QUdpSocket *udpSocket;
    Ui::Form *ui;

    int cd;

    static const int fieldWidth = 10;
    static const int fieldHeight = 16;
    static const int cellSize = 24;
    static const int fallingSpeed = 300;

    int playerGameField[fieldHeight][fieldWidth];
    int friendGameField[fieldHeight][fieldWidth];

    QString token;
    QVector<QString> lobbies;
    QString lobby;

    QString game_id;
    QString join_token;
    QString create_token;
    QString creator;
    QString server_ip;

    QString message;
    QString submessage;
    bool gameIsOver;

    QString opponentName;

    QBasicTimer _timer;
    QBasicTimer _countdown;

    QNetworkAccessManager * pManager;
    QNetworkAccessManager * gameManager;
    WSClient * wsclient;

    QJsonArray openGames;

    void addQuad(int x, int y);
    void addRectBlock();
    void addZBlock();
    void addTBlock();
    void addLBlock();
    void addLMirrorBlock();
    void addZMirrorBlock();
    void addMinusBlock();

    void drawField(int field[][fieldWidth], int offsetX);
    void drawMessage();

    void updateField(int field[][fieldWidth]);
    void moveIfCan(int field[][fieldWidth], int offsetX);
    void rotateIfCan(int field[][fieldWidth]);

    void blockLanded(int field[][fieldWidth]);
    void checkLines(int field[][fieldWidth]);
    void gameOver(bool loose);

    void debugArray(int arr[][3]);
    void setMessage(QString message);
    void setSubmessage(QString message);
    void opponentFound();
    void Ready();
    void newGame();
    void authorize(QString address, QString login, QString password);
    void hideControls();
    void showControls();

    void findGame();
    void createGame();
    void connectToGameServer();
public:
    explicit Form(QWidget *parent = 0);
    ~Form();

private slots:
    void opponentConnected(QString name);
    void opponentLost();
    void authorizeBtnClicked();
    void registerBtnClicked();
    void dropRandomBlock();
    void processPendingDatagrams();

    void authorizationReply(QNetworkReply *reply);
    void gamesFound(QNetworkReply *reply);
    void gameCreated(QNetworkReply *reply);

    void connectionRefused();
    void connected();
    void updateOpponentsField(int field[][10]);
    void serverRefused();
protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *ev);
};

#endif // FORM_H
