#include "wsclient.h"


WSClient::WSClient(const QUrl &url, QString auth_token, QString game_id, QString game_token, QObject *parent) :
    QObject(parent),
    m_url(url)
{
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 10; j++) {
            field[i][j] = 0;
        }
    this->auth_token = auth_token;
    this->game_id = game_id;
    this->game_token = game_token;
    connect(&m_webSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_webSocket, SIGNAL(disconnected()), this, SLOT(closedWS()));
}

void WSClient::Open()
{
    m_webSocket.open(QUrl(m_url));
}

void WSClient::closedWS()
{
    m_webSocket.close();
    emit refused();
}

void WSClient::onConnected()
{
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &WSClient::onTextMessageReceived);
    QString json = QString("{   \"msg_type\" : \"handshake\",\
                              \"auth_token\" : \"%1\", \
                                 \"game_id\" : \"%2\", \
                              \"game_token\" : \"%3\" }").arg(auth_token, game_id, game_token);
    m_webSocket.sendTextMessage(json.toUtf8());
}

void WSClient::onTextMessageReceived(QString message)
{
    QByteArray data;
    data.append(message);
    qDebug() << "recieved msg: " << data << "\n";
    QJsonDocument json = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = json.object();

    QString msg_type = jsonObject["msg_type"].toString();

    if (msg_type == "opponent_connected")
    {
        QString user = jsonObject["opponent"].toString();
        emit opponent_connected(user);
    }
    else if (msg_type == "lost")
    {
        emit opponent_lost();
    }
    else if (msg_type == "game")
    {
        QJsonArray array = jsonObject["field"].toArray();
        int x = 0;
        foreach (const QJsonValue & value, array) {
            QJsonObject obj = value.toObject();
            field[x / 10][x % 10] = obj["n"].toInt();
            x++;
        }
        emit opponent_moved_block();
    }
}

void WSClient::sendGameOver()
{
    QString json = "{ \"msg_type\" : \"lost\" }";
    m_webSocket.sendTextMessage(json.toUtf8());
}

void WSClient::sendGameField(int field[][fieldWidth])
{
    QString json = "{ \"msg_type\" : \"game\",\
                   \"field\" : [ ";
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 10; j++)
        {
            json += " { \"n\" : " + QString::number(field[i][j]) + " } ";
            if (i != 15 || j != 9)
            {
                json += ",";
            }
        }
    json += " ] }";
    m_webSocket.sendTextMessage(json.toUtf8());
}

WSClient::~WSClient()
{

}

