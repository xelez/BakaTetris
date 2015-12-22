#include "tetrisscene.h"
#include <QDebug>

TetrisScene::TetrisScene(QString server_ip)
{
    this->_io = new sio::client();
    this->server_ip = server_ip;
    for (int i = 0; i < fieldHeight + 3; i++)
        for (int j = 0; j < fieldWidth; j++) {
            this->playerGameField[i][j] = 0;
            this->friendGameField[i][j] = 0;
        }
    this->gameIsOver = false;
    dropRandomBlock();
}

TetrisScene::~TetrisScene()
{

}

void TetrisScene::dropRandomBlock()
{
    int block_type = qrand() % (4 + 1);
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
    }
}

void TetrisScene::addMinusBlock()
{
    int x = TetrisScene::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x+2, 0);
}

void TetrisScene::addLBlock()
{
    int x = TetrisScene::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x+1, 1);
}

void TetrisScene::addTBlock()
{
    int x = TetrisScene::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x+1, 0);
    addQuad(x, 1);
}

void TetrisScene::addZBlock()
{
    int x = TetrisScene::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x, 1);
    addQuad(x+1, 1);
}

void TetrisScene::addRectBlock()
{
    int x = TetrisScene::fieldWidth / 2;
    addQuad(x-1, 0);
    addQuad(x, 0);
    addQuad(x-1, 1);
    addQuad(x, 1);
}

void TetrisScene::addQuad(int x, int y)
{
    if (this->friendGameField[y][x] != 0)
        gameOver();
    this->friendGameField[y][x] = 3;
}

void TetrisScene::gameOver()
{
    this->gameIsOver = true;
}

void TetrisScene::keyPressEvent(QKeyEvent *event)
{

}
