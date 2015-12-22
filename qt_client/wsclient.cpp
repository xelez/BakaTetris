#include "wsclient.h"


WSClient::WSClient(const QUrl &url, QString auth_token, QString game_id, QString game_token, QObject *parent) :
    QObject(parent),
    m_url(url)
{
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
    /*QJsonDocument json = QJsonDocument::fromJson(message);
    QJsonObject jsonObject = json.object();
    QString msg_type = jsonObject["msg_type"].toString();
    if (msg_type == "opponent_connected")
    {

    }
    else if (msg_type == "opponent_connected")
    {

    }
    server_ip  = jsonObject["server_ip"].toString();*/
    //create_token = jsonObject["create_token"].toString();
}

WSClient::~WSClient()
{

}

