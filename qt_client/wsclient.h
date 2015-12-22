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
    explicit WSClient(const QUrl &url, QString auth_token, QString game_id, QString game_token, QObject *parent = Q_NULLPTR);
    void Open();
    ~WSClient();

Q_SIGNALS:
    void refused();
    void connected();
    void opponent_connected();
    void opponent_moved_block(int[][16]);

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
};

#endif // WSCLIENT_H
