#include <QDebug>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QListView>
#include <QPainter>

#include "main_window.h"
#include "application.h"

TMainWindow::TMainWindow(TImageStorage* imageStorage, QAbstractItemModel* friendListModel)
    : QWidget(NULL)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(friendListModel && "missing friendListModel");
    this->setGeometry(QDesktopWidget().availableGeometry().center().x() - (MAIN_WINDOW_WIDTH / 2),
                      QDesktopWidget().availableGeometry().center().y() - (MAIN_WINDOW_HEIGHT / 2),
                       MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    QVBoxLayout* currentLayout = new QVBoxLayout(this);
    QListView* friendListView = new QListView();
    friendListView->setModel(friendListModel);
    friendListView->setItemDelegate(new TFriendItemDelegate(imageStorage));
    currentLayout->addWidget(friendListView);

    this->show();
}


TFriendItemDelegate::TFriendItemDelegate(TImageStorage* imageStorage)
    : ImageStorage(imageStorage)
{
}

void TFriendItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    // QString line0 = index.model()->data(index.model()->index(index.row(), 1)).toString();
    if (!index.data().canConvert<TFriendData>()) {
        return;
    }
    TFriendData frnd = index.data().value<TFriendData>();
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QRect rect = opt.rect;
    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // set pen color
    if (opt.state & QStyle::State_Selected) {
        painter->save();
        QBrush selectionBrush(Qt::blue);
        painter->setBrush(selectionBrush);
        painter->drawRect(rect.adjusted(1, 1, -1, -1));
        //painter->drawRoundedRect(rect.adjusted(1,1,-1,-1), 5, 5);
        painter->restore();

        painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(opt.palette.color(cg, QPalette::Text));
    }

    // draw 2 lines of text
    QFont font;
    font.setPixelSize(MIN_FRIEND_ITEM_HEIGHT - 6);
    painter->setFont(font);
    painter->drawImage(rect.left(), rect.top() + 2, ImageStorage->GetStatusImage(frnd.Status));
    painter->drawText(QRect(rect.left() + 18, rect.top(), rect.width() - 16, rect.height()),
                      opt.displayAlignment, frnd.Login);
}

QSize TFriendItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(MIN_FRIEND_ITEM_HEIGHT);
    return size;
}
