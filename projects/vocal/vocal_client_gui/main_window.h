#pragma once

#include <QWidget>
#include <QAbstractItemModel>

const size_t MAIN_WINDOW_WIDTH = 120;
const size_t MAIN_WINDOW_HEIGHT = 500;

class TMainWindow: public QWidget {
    Q_OBJECT
public:
    explicit TMainWindow(QAbstractItemModel* friendListModel);
};
