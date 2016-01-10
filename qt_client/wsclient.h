#ifndef WSCLIENT_H
#define WSCLIENT_H

#include <QtCore/QObject>
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class WSClient : public QObject
{
Q_OBJECT
public:
    static const int fieldWidth = 10;
    static const int fieldHeight = 16;
    explicit WSClient(const QUrl &url, QString auth_token, QString game_id, QString game_token, QObject *parent = Q_NULLPTR);
    void Open();
    void sendGameOver();
    void sendGameField(int field[][fieldWidth]);
    ~WSClient();

Q_SIGNALS:
    void refused();
    void connected();
    void opponent_connected(QString name);
    void opponent_lost();
    void opponent_moved_block(int [][10]);

private Q_SLOTS:
    void closedWS();
    void onConnected();
    void onTextMessageReceived(QString message);

private:
    QWebSocket m_webSocket;

    QString auth_token;
    QString game_id;
    QString game_token;

    QUrl m_url;
    int field[16][10];
};

#endif // WSCLIENT_H
