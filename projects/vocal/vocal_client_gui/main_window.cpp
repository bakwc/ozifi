#include <QDebug>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QListView>

#include "main_window.h"

TMainWindow::TMainWindow(QAbstractItemModel* friendListModel)
    : QWidget(NULL)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(friendListModel && "missing friendListModel");
    this->setBaseSize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    this->setGeometry(QDesktopWidget().availableGeometry().center().x() - (this->width() / 2),
                      QDesktopWidget().availableGeometry().center().y() - (this->height() / 2),
                       this->width(), this->height());
    QVBoxLayout* currentLayout = new QVBoxLayout(this);
    QListView* friendListView = new QListView();
    friendListView->setModel(friendListModel);
    currentLayout->addWidget(friendListView);

    this->show();
}
