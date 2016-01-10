#include "form.h"
#include "ui_form.h"
#include <QDebug>
#include <QKeyEvent>
#include <QTime>
#include <QMessageBox>

Form::Form(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    ui->passwordTxt->setEchoMode(QLineEdit::Password);
    ui->passwordTxt->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);

    connect(ui->loginBtn, SIGNAL(clicked()), this, SLOT(authorizeBtnClicked()));
    connect(ui->regBtn, SIGNAL(clicked()), this, SLOT(registerBtnClicked()));

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, 13333);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    qsrand((uint)QTime::currentTime().msec());
    setMessage("Authorization");
    //UDP
    QHostAddress group = QHostAddress("224.0.0.111");
    QByteArray datagram = "hello";
    udpSocket->writeDatagram(datagram, group, 13334);

    //REST
    pManager = new QNetworkAccessManager();
    connect(pManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(authorizationReply(QNetworkReply*)));

    gameManager = new QNetworkAccessManager();
}

void Form::authorize(QString address, QString login, QString password)
{
    setSubmessage("");
    QUrl url("http://" + address);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    pManager->post( request, QString("user=" + login + "&password=" + password).toUtf8());
}

void Form::hideControls()
{
    this->ui->loginBtn->hide();
    this->ui->regBtn->hide();
    this->ui->loginTxt->hide();
    this->ui->passwordTxt->hide();
}

void Form::showControls()
{
    this->ui->loginBtn->show();
    this->ui->regBtn->show();
    this->ui->loginTxt->show();
    this->ui->passwordTxt->show();
}

void Form::authorizationReply(QNetworkReply * reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray temp = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(temp);
        QJsonObject jsonObject = json.object();
        this->token = jsonObject["token"].toString();
        //got token
        hideControls();
        findGame();
    }
    else {
        setSubmessage(reply->errorString());
    }
}

void Form::authorizeBtnClicked()
{
    QString login = ui->loginTxt->text().toUtf8();
    QString password = ui->passwordTxt->text().toUtf8();
    lobby = lobbies[rand() % lobbies.size()];
    authorize(lobby + "/signin", login, password);
}

void Form::registerBtnClicked()
{
    QString login = ui->loginTxt->text().toUtf8();
    QString password = ui->passwordTxt->text().toUtf8();
    lobby = lobbies[rand() % lobbies.size()];
    authorize(lobby + "/signup", login, password);
}

Form::~Form()
{
    delete ui;
}

void Form::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int offsetX = 40;
    int offsetX2 = 320;

    if (message.length() > 0){
        drawMessage();
    }
    else {
        drawField(this->playerGameField, offsetX);
        drawField(this->friendGameField, offsetX2);
    }
}

void Form::findGame()
{
    connect(gameManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gamesFound(QNetworkReply*)));
    disconnect(gameManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gameCreated(QNetworkReply*)));

    setSubmessage("");
    setMessage("Looking for opponent...");
    QUrl url("http://" + lobby + "/games/open?token=" + token);
    QNetworkRequest request(url);
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    gameManager->get(request);
}

void Form::newGame()
{
    cd = 4;
    for (int i = 0; i < fieldHeight; i++)
        for (int j = 0; j < fieldWidth; j++) {
            this->playerGameField[i][j] = 0;
            this->friendGameField[i][j] = 0;
        }
    this->gameIsOver = false;
}

void Form::gamesFound(QNetworkReply * reply)
{
    qDebug() << "gamesFound";
    setSubmessage("");
    setMessage("Looking for opponent...");
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray temp = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(temp);
        openGames = json.array();
        connectToGameServer();
    }
    else {
        qDebug() << reply->errorString();
        connectToGameServer();
    }
}

void Form::connectToGameServer()
{
    qDebug() << "connectToGameServer";
    if (openGames.count() == 0){
        createGame();
        return;
    }
    foreach (const QJsonValue & val, openGames) {
        QJsonObject obj = val.toObject();
        game_id    = obj["id"].toString();
        creator    = obj["creator"].toString();
        server_ip  = obj["server_ip"].toString();
        join_token = obj["join_token"].toString();
    }
    if (wsclient)
        delete wsclient;
    openGames.erase(openGames.begin());

    wsclient = new WSClient(QUrl("ws://" + server_ip), token, game_id, join_token);
    connect(wsclient, SIGNAL(refused()), this, SLOT(connectionRefused()));
    connect(wsclient, SIGNAL(connected()), this, SLOT(connected()));
    connect(wsclient, SIGNAL(opponent_connected()), this, SLOT(opponentConnected()));
    connect(wsclient, SIGNAL(opponent_moved_block(int[][16])), this, SLOT(updateOpponentsField(int[][16])));
    wsclient->Open();
}

void Form::connectionRefused()
{
    qDebug() << "refused";
    connectToGameServer();
}

void Form::connected()
{
    qDebug() << "connected to game";
}

void Form::createGame()
{
    qDebug() << "create game";
    disconnect(gameManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gamesFound(QNetworkReply*)));
    connect(gameManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gameCreated(QNetworkReply*)));

    QUrl url("http://" + lobby + "/games");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    gameManager->post( request, QString("token=" + token).toUtf8());
}

void Form::gameCreated(QNetworkReply * reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "game created";
        QByteArray temp = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(temp);
        QJsonObject jsonObject = json.object();
        game_id    = jsonObject["game_id"].toString();
        server_ip  = jsonObject["server_ip"].toString();
        create_token = jsonObject["create_token"].toString();

        wsclient = new WSClient(QUrl("ws://" + server_ip), token, game_id, create_token);
        connect(wsclient, SIGNAL(refused()), this, SLOT(serverRefused()));
        connect(wsclient, SIGNAL(opponent_connected()), this, SLOT(opponentConnected()));
        connect(wsclient, SIGNAL(opponent_moved_block(int[][16])), this, SLOT(updateOpponentsField(int[][16])));
        wsclient->Open();
    }
    else {
        setSubmessage("Server is busy");
    }
}

void Form::serverRefused()
{
    qDebug() << "server refused";
    findGame();
}

void Form::updateOpponentsField(int field[][16])
{
    for (int r = 0; r < fieldHeight; r++)
        for (int c =0; c < fieldWidth; c++)
            this->friendGameField[r][c] = field[r][c];
}

void Form::opponentConnected()
{
    qDebug() << "gameStart!!!";
    setMessage("");
    newGame();
    this->setFocus();
    dropRandomBlock();
}

void Form::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress from;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &from);

        QString server(datagram.data());
        this->lobbies.push_back(server);
    }
}

void Form::opponentFound()
{
    setMessage("Opponent found!");
    _countdown.start(1000, this);
}

void Form::drawMessage()
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap));

    QFont font = painter.font();
    font.setPixelSize(30);
    painter.setFont(font);
    painter.drawText(0, 0, 600, 200,
                     Qt::AlignHCenter | Qt::AlignVCenter,
                     message);
    font.setPixelSize(12);
    painter.setFont(font);
    if (submessage.length() > 0)
        painter.drawText(0, 200, 600, 200,
                         Qt::AlignHCenter | Qt::AlignVCenter,
                         submessage);
}

void Form::setMessage(QString message)
{
    this->message = message;
    update();
}

void Form::setSubmessage(QString message)
{
    this->submessage = message;
    update();
}

void Form::drawField(int field[][fieldWidth], int offsetX)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap));
    painter.drawRect(offsetX, 8, cellSize * fieldWidth, cellSize * fieldHeight);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap));
    for (int i = 0; i < fieldHeight; i++)
        for (int j = 0; j < fieldWidth; j++) {
            if (field[i][j] == 1)
            {
                painter.setBrush(Qt::black);
                painter.drawRect(offsetX + j * cellSize, 8 + i * cellSize, cellSize, cellSize);
            }
            else if (field[i][j] == 2)
            {
                painter.setBrush(Qt::white);
                painter.drawRect(offsetX + j * cellSize, 8 + i * cellSize, cellSize, cellSize);
            }
        }
}

void Form::dropRandomBlock()
{
    if (this->gameIsOver)
        return;
    qDebug() << "drop random block";
    _timer.start(this->fallingSpeed, this);

    int block_type = qrand() % (6 + 1);
    switch (block_type)
    {
        case 0:
            addMinusBlock();
        break;
        case 1:
            addLBlock();
        break;
        case 2:
            addTBlock();
        break;
        case 3:
            addRectBlock();
        break;
        case 4:
            addZBlock();
        break;
        case 5:
            addZMirrorBlock();
        break;
        case 6:
            addLMirrorBlock();
        break;
    }
}

void Form::addMinusBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x+2, 0);
}

void Form::addLBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x+1, 1);
}

void Form::addLMirrorBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x-1, 1);
}

void Form::addTBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x, 1);
}

void Form::addZBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x, 1);
    addQuad(x+1, 1);
}

void Form::addZMirrorBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x+1, 0);
    addQuad(x, 0);
    addQuad(x, 1);
    addQuad(x-1, 1);
}

void Form::addRectBlock()
{
    int x = Form::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x-1, 1);
    addQuad(x, 1);
}

void Form::addQuad(int x, int y)
{
    if (this->playerGameField[y][x] != 0 && !this->gameIsOver)
        gameOver(true);
    this->playerGameField[y][x] = 2;
}


void Form::gameOver(bool loose)
{
    qDebug() << "game over";
    this->gameIsOver = true;
    _timer.stop();
    if (loose)
        setMessage("You lost");
    else
        setMessage("You won!");
    setSubmessage("--press Enter to find new game--");

}

void Form::Ready()
{
    setMessage("");
}

void Form::timerEvent(QTimerEvent * ev) {
    if (ev->timerId() == _timer.timerId()) {
        updateField(this->playerGameField);
        update();
    }
    else if (ev->timerId() == _countdown.timerId()) {
        cd--;
        if (cd != 0)
            setMessage("Games starts in " + QString::number(cd));
        else
            Ready();
    }
    else {
        QWidget::timerEvent(ev);
        return;
    }
}

void Form::updateField(int field[][fieldWidth])
{
    for (int i = 0; i < fieldHeight; i++)
        for (int j = 0; j < fieldWidth; j++) {
            if( field[i][j] == 2)
            {
                if (i == fieldHeight - 1 || field[i+1][j] == 1){
                    blockLanded(field);
                    return;
                }
            }
        }
    for (int i = fieldHeight - 1; i >= 0; i--)
        for (int j = 0; j < fieldWidth; j++) {
            if( field[i][j] == 2)
            {
                field[i][j] = 0;
                field[i+1][j] = 2;
            }
        }
}

void Form::blockLanded(int field[][fieldWidth])
{
    if (this->gameIsOver)
        return;
    qDebug() << "block landed";
    for (int i = 0; i < fieldHeight; i++)
        for (int j = 0; j < fieldWidth; j++)
            if( field[i][j] == 2)
            {
                field[i][j] = 1;
            }
    _timer.stop();
    checkLines(field);
    dropRandomBlock();
}

void Form::checkLines(int field[][fieldWidth])
{
    for (int i = 0; i < fieldHeight; i++){
        bool clearLine = true;
        for (int j = 0; j < fieldWidth; j++)
            if( field[i][j] != 1)
                clearLine = false;
        if (clearLine) {
            for (int x = 0; x < fieldWidth; x++)
                field[i][x] = 0;
            for (int y = i; y > 0; y--) {
                for (int x = 0; x < fieldWidth; x++)
                    field[y][x] = field[y-1][x];
            }
        }
    }
}

void Form::keyPressEvent(QKeyEvent *event)
{
    if (this->gameIsOver)
        return;
    qDebug() << "key_press" << "\n";
    switch(event->key())
    {
        case Qt::Key_Left:
            moveIfCan(this->playerGameField, -1);
        break;
        case Qt::Key_Right:
            moveIfCan(this->playerGameField, 1);
        break;
        case Qt::Key_Down:
            updateField(this->playerGameField);
            update();
        break;
        case Qt::Key_Space:
            rotateIfCan(this->playerGameField);
        break;
    }
}

void Form::rotateIfCan(int field[][fieldWidth])
{
    int x = fieldWidth;
    int y = fieldHeight;
    int width = 0;
    int height = 0;
    for (int i = 0; i < fieldHeight; i++)
        for (int j = 0; j < fieldWidth; j++)
            if (field[i][j] == 2)
            {
                if (i < y)
                    y = i;
                if (j < x)
                    x = j;
                if (i > height)
                    height = i;
                if (j > width)
                    width = j;
            }
    width = width - x + 1;
    height = height - y + 1;
    if (width > 0 && height > 0)
    {
        int size = std::max(width, height);
        if (size == 3)
        {
            int a[3][3];
            int b[3][3];
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (field[y + i][x + j] == 2)
                        a[i][j] = field[y + i][x + j];
                    else
                        a[i][j] = 0;

            if (width > height)
            {
                if (y > fieldHeight - 3)
                    return;
            }
            else {
                if (x > fieldWidth - 3)
                    return;
            }

            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    b[c][3 - 1 - r] = a[r][c];

            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    if (b[r][c] == 2 && field[y + r][x + c] == 1)
                        return;

            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    if (a[r][c] == 2)
                        field[y + r][x + c] = 0;

            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    if (b[r][c] == 2)
                        field[y + r][x + c] = 2;

        }
        if (size == 4)
        {
            if (width > height)
            {
                if (y + 3 >= fieldHeight || y == 0)
                    return;
                if (field[y + 1][x] == 0 && field[y + 2][x] == 0)
                {
                    field[y][x] = 0;
                    field[y][x + 2] = 0;
                    field[y][x + 3] = 0;

                    field[y - 1][x + 1] = 2;
                    field[y + 1][x + 1] = 2;
                    field[y + 2][x + 1] = 2;
                }
            }
            else {
                if (x == 0 || x > 7)
                    return;
                if (field[y + 1][x - 1] == 0 && field[y + 1][x + 1] == 0 &&
                        field[y + 1][x + 2] == 0)
                {
                    field[y][x] = 0;
                    field[y + 2][x] = 0;
                    field[y + 3][x] = 0;

                    field[y + 1][x - 1] = 2;
                    field[y + 1][x + 1] = 2;
                    field[y + 1][x + 2] = 2;
                }
            }
        }
    }
    update();
}

void Form::moveIfCan(int field[][fieldWidth], int offsetX)
{
    for (int i = 0; i < fieldHeight; i++)
        for (int j = 0; j < fieldWidth; j++)
            if (field[i][j] == 2 &&
               (j + offsetX < 0 || j + offsetX > fieldWidth - 1
                     || field[i][j + offsetX] == 1))
                return;
    if (offsetX > 0)
        for (int i = 0; i < fieldHeight; i++)
            for (int j = fieldWidth - 1; j >= 0; j--) {
                if( field[i][j] == 2)
                {
                    field[i][j] = 0;
                    field[i][j + 1] = 2;
                }
            }
    else
        for (int i = 0; i < fieldHeight; i++)
            for (int j = 0; j < fieldWidth; j++) {
                if( field[i][j] == 2)
                {
                    field[i][j] = 0;
                    field[i][j - 1] = 2;
                }
            }
    update();
}

void Form::debugArray(int arr[][3])
{
    for (int i = 0; i < 3; i++)
    {
        qDebug() << arr[i][0] << arr[i][1] << arr[i][2];
    }
}
