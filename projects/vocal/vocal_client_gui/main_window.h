#pragma once

#include <QWidget>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>

const size_t MAIN_WINDOW_WIDTH = 220;
const size_t MAIN_WINDOW_HEIGHT = 500;
const size_t MIN_FRIEND_ITEM_HEIGHT = 20;

class TImageStorage;
class TFriendItemDelegate: public QStyledItemDelegate {
public:
    TFriendItemDelegate(TImageStorage* imageStorage);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
private:
    TImageStorage* ImageStorage;
};

class TMainWindow: public QWidget {
    Q_OBJECT
public:
    explicit TMainWindow(TImageStorage *imageStorage, QAbstractItemModel* friendListModel);
};
