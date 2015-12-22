#include <QApplication>
#include <QGraphicsView>

#include "form.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Form view;
    view.setGeometry(200, 200, 600, 400);
    view.setMinimumSize(600, 400);
    view.setMaximumSize(600, 400);
    view.show();

    return a.exec();
}
