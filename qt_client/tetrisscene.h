#ifndef TETRISSCENE_H
#define TETRISSCENE_H

#include <QObject>
#include <QGraphicsScene>

class TetrisScene : public QGraphicsScene
{
private:
    static const int fieldWidth = 10;
    static const int fieldHeight = 16;
    static const int fallingSpeed = 10;

    int playerGameField[fieldHeight][fieldWidth];
    int friendGameField[fieldHeight][fieldWidth];

    QString server_ip;
    sio::client * _io;

    bool gameIsOver;

    void addQuad(int x, int y);
    void addRectBlock();
    void addZBlock();
    void addTBlock();
    void addLBlock();
    void addMinusBlock();

    void gameOver();
public:
    TetrisScene(QString server_ip);
    ~TetrisScene();

public Q_SLOTS:
    void dropRandomBlock();

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // TETRISSCENE_H
