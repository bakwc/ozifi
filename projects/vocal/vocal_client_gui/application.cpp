#include <QDebug>
#include <QPoint>
#include <QMessageBox>

#include "application.h"

#include <thread>

using namespace std;
using namespace std::placeholders;

TVocaGuiApp::TVocaGuiApp(int &argc, char** argv)
    : QApplication(argc, argv)
    , Status(ST_None)
{
    qRegisterMetaType<NVocal::ECallStatus>("ECallStatus");

    NVocal::TClientConfig config;
    config.CaptchaAvailableCallback = std::bind(&TVocaGuiApp::OnCaptcha, this, _1);
    config.RegisterResultCallback = std::bind(&TVocaGuiApp::OnRegistered, this, _1);
    config.LoginResultCallback = std::bind(&TVocaGuiApp::OnLogined, this, _1);
    config.ConnectedCallback = std::bind(&TVocaGuiApp::OnConnected, this, _1);
    config.OnFriendAdded = std::bind(&TVocaGuiApp::OnFriendAdded, this, _1);
    config.OnFriendRemoved = std::bind(&TVocaGuiApp::OnFriendRemoved, this, _1);
    config.OnFriendUpdated = std::bind(&TVocaGuiApp::OnFriendUpdated, this, _1);
    config.OnMessageReceived = std::bind(&TVocaGuiApp::OnMessageReceived, this, _1);
    config.OnFriendCallStatusChanged = [this](NVocal::TFriendRef frnd) {
        emit OnFriendCallStatusChanged(QString::fromStdString(frnd->GetLogin()),
                                       frnd->GetCallStatus());
    };

    config.FriendRequestCallback = [this](const std::string& login) {
        QString question = tr("Accept friend request from <b>%1</b>")
                              .arg(QString::fromStdString(login));
        auto reply = QMessageBox::question(nullptr, tr("Friend request"), question,
                                           QMessageBox::Yes, QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            Client->AddFriend(login);
        } else {
            Client->RemoveFriend(login);
        }
    };

    config.AudioInput = std::bind(&TVocaGuiApp::OnAudioInput, this, _1);
    config.StateDir = "data";
    connect(this, &TVocaGuiApp::RegistrationSuccess,
            this, &TVocaGuiApp::OnSuccesfullyRegistered);
    ChatWindows.reset(new TChatWindows());
    connect(this, &TVocaGuiApp::MessageReceived,
            ChatWindows.get(), &TChatWindows::ShowMessage);
    connect(ChatWindows.get(), &TChatWindows::SendMessage,
            this, &TVocaGuiApp::SendMessage);
    connect(this, &TVocaGuiApp::OnFriendCallStatusChanged,
            ChatWindows.get(), &TChatWindows::OnFriendCallStatusChanged);
    connect(ChatWindows.get(), &TChatWindows::OnStartCall, [this](const QString& login) {
        try {
            NVocal::TFriendRef frnd = Client->GetFriend(login.toStdString());
            frnd->StartCall(false);
        } catch (const UException&) {
            emit OnFriendCallStatusChanged(login, NVocal::CAS_NotCalling);
        }
    });
    connect(ChatWindows.get(), &TChatWindows::OnFinishCall, [this](const QString& login) {
        try {
            NVocal::TFriendRef frnd = Client->GetFriend(login.toStdString());
            frnd->FinishCall();
        } catch (const UException&) {
            emit OnFriendCallStatusChanged(login, NVocal::CAS_NotCalling);
        }
    });
    Client.reset(new NVocal::TClient(config));
    if (Client->HasConnectData()) {
        LaunchMain();
    } else {
        LaunchLogin();
    }
    Audio.reset(new TAudio());
}

TVocaGuiApp::~TVocaGuiApp() {
    qApp;
}

QString TVocaGuiApp::SelfLogin() {
    return QString::fromStdString(Client->GetFullLogin());
}

void TVocaGuiApp::Authorize() {
    try {
        Client->Connect();
    } catch (const UException& e) {
        cout << "could not authorize: " << e.what() << "\n";
        _exit(42);
    }
}

void TVocaGuiApp::OnCaptcha(const TBuffer& data) {
    QImage captchaImage;
    captchaImage.loadFromData((const unsigned char*)data.Data(), data.Size());
    emit CaptchaAvailable(captchaImage);
}

void TVocaGuiApp::OnRegistered(NVocal::ERegisterResult res) {
    if (res == NVocal::RR_Success) {
        emit RegistrationSuccess();
    } else {
        QString msg = QString::fromStdString(RegisterResultToString(res));
        emit RegistrationFailed(msg);
    }
}

void TVocaGuiApp::OnLogined(NVocal::ELoginResult res) {
    if (res == NVocal::LR_Success) {
        emit RegistrationSuccess();
    } else {
        QString msg = QString::fromStdString(LoginResultToString(res));
        emit RegistrationFailed(msg);
    }
}

void TVocaGuiApp::OnConnected(bool success) {
    if (!success) {
        cout << "failed to authorize\n";
    }
    cout << "authorized\n";
}

void TVocaGuiApp::OnMessageReceived(const NVocal::TMessage& message) {
    bool incoming = true;
    if (message.From == Client->GetFullLogin()) { // todo: move to core
        incoming = false;
    }
    QString frndLogin = incoming ? QString::fromStdString(message.From) : QString::fromStdString(message.To);
    QString messageText = QString::fromStdString(message.Text);
    emit MessageReceived(frndLogin, messageText, incoming);
}

void TVocaGuiApp::Register(const QString& login) {
    assert(Status == ST_None && "Wrong state");
    Status = ST_Registering;
    std::thread([this, login]() {
        Client->Register(login.toStdString());
    }).detach();
}

void TVocaGuiApp::Login(const QString& login) {
    assert(Status == ST_None && "Wrong state");
    Status = ST_Logining;
    std::thread([this, login]() {
        Client->Login(login.toStdString());
    }).detach();
}

void TVocaGuiApp::SendMessage(const QString& frndLogin, const QString& message) {
    Client->GetFriend(frndLogin.toStdString())->SendMessage(message.toStdString());
}

void TVocaGuiApp::LaunchLogin() {
    LoginWindow.reset(new TLoginWindow());
    connect(LoginWindow.get(), &TLoginWindow::Register,
            this, &TVocaGuiApp::Register);
    connect(LoginWindow.get(), &TLoginWindow::Login,
            this, &TVocaGuiApp::Login);
    connect(this, &TVocaGuiApp::CaptchaAvailable,
            LoginWindow.get(), &TLoginWindow::OnCaptchaAvailable);
    connect(LoginWindow.get(), &TLoginWindow::DoLogin,
            [this] (const QString& captcha, const QString& password)
    {
       Client->Login(password.toStdString(), captcha.toStdString());
    });
    connect(LoginWindow.get(), &TLoginWindow::DoRegister,
            [this] (const QString& captcha, const QString& password, const QString& email)
    {
        Client->Register(password.toStdString(),
                         email.toStdString(),
                         captcha.toStdString());
    });
    connect(this, &TVocaGuiApp::RegistrationFailed,
            LoginWindow.get(), &TLoginWindow::OnRegistrationFailed);
}

void TVocaGuiApp::LaunchMain() {
    FriendListModel.reset(new TFriendListModel(*Client));
    MainWindow.reset(new TMainWindow(&ImageStorage, FriendListModel.get()));
    connect(MainWindow.get(), &TMainWindow::FriendDoubleClicked, ChatWindows.get(), &TChatWindows::ShowChatWindow);
    connect(MainWindow.get(), &TMainWindow::AddFriendClicked, [this]() {
        QPoint pos(-1, -1);
        if (AddFriendWindow) {
            pos = AddFriendWindow->pos();
        }
        AddFriendWindow.reset(new TAddFriendWindow());
        if (pos.x() != -1) {
            AddFriendWindow->move(pos);
        }
        connect(AddFriendWindow.get(), &TAddFriendWindow::OnAddFriend, [this](const QString& login) {
            try {
                Client->AddFriend(login.toStdString());
            } catch(const UException&) {
                // todo: handle somehow
            }
        });
    });
    Client->Connect();
}

void TVocaGuiApp::OnSuccesfullyRegistered() {
    Q_ASSERT(Client->HasConnectData() && "no data to connect");
    LoginWindow.reset(nullptr);
    LaunchMain();
}

void TVocaGuiApp::OnFriendAdded(NVocal::TFriendRef frnd) {
    if (FriendListModel) {
        FriendListModel->OnFriendAdded(frnd);
    }
}

void TVocaGuiApp::OnFriendRemoved(NVocal::TFriendRef frnd) {
    if (FriendListModel) {
        FriendListModel->OnFriendRemoved(frnd);
    }
}

void TVocaGuiApp::OnFriendUpdated(NVocal::TFriendRef frnd) {
    if (FriendListModel) {
        FriendListModel->OnFriendUpdated(frnd);
    }
}

string TVocaGuiApp::OnAudioInput(size_t size) {
    string data;
    data.resize(size);
    return data;
}

TFriendListModel::TFriendListModel(NVocal::TClient& vocalClient)
    : VocalClient(vocalClient)
{
    for (NVocal::TFriendIterator it = VocalClient.FriendsBegin(); it != VocalClient.FriendsEnd(); ++it) {
        Friends.push_back(it->second);
    }
}


int TFriendListModel::rowCount(const QModelIndex&) const {
    return Friends.size();
}

QVariant TFriendListModel::data(const QModelIndex& index, int role) const {
    QVariant result;
    if (role == Qt::DisplayRole && index.row() < Friends.size()) {
        const NVocal::TFriendRef& frnd = Friends[index.row()];
        TFriendData frndData;
        frndData.Login = QString::fromStdString(frnd->GetLogin());
        switch (frnd->GetStatus()) {
        case NVocal::FS_AddRequest:
        case NVocal::FS_Unauthorized: frndData.Status = FS_Unauthorized; break;
        case NVocal::FS_Offline: frndData.Status = FS_Offline; break;
        case NVocal::FS_Busy: frndData.Status = FS_Busy; break;
        case NVocal::FS_Away: frndData.Status = FS_Away; break;
        case NVocal::FS_Available: frndData.Status = FS_Available; break;
        default: Q_ASSERT(!"unexpected status");
        }
        result.setValue(frndData);
    }
    return result;
}

void TFriendListModel::OnFriendAdded(NVocal::TFriendRef frnd) {
    beginInsertRows(QModelIndex(), Friends.size(), Friends.size());
    Friends.push_back(frnd);
    endInsertRows();
}

void TFriendListModel::OnFriendRemoved(NVocal::TFriendRef frnd) {
    // todo: rewrite using hash
    for (size_t i = 0; i < Friends.size(); ++i) {
        if (frnd == Friends[i]) {
            beginRemoveRows(QModelIndex(), i, i + 1);
            Friends.remove(i);
            endRemoveRows();
            break;
        }
    }
}

void TFriendListModel::OnFriendUpdated(NVocal::TFriendRef frnd) {
    // todo: rewrite using hash for quick friend access
    for (size_t i = 0; i < Friends.size(); ++i) {
        if (frnd == Friends[i]) {
            emit dataChanged(index(i), index(i));
            break;
        }
    }
}

TImageStorage::TImageStorage() {
    StatusImages.resize(FS_Count);
    StatusImages[FS_Unauthorized].load(":/icons/user-offline.png");
    StatusImages[FS_Busy].load(":/icons/user-extended-away.png");
    StatusImages[FS_Away].load(":/icons/user-away.png");
    StatusImages[FS_Offline].load(":/icons/user-offline.png");
    StatusImages[FS_Available].load(":/icons/user-available.png");
}

const QImage& TImageStorage::GetStatusImage(EFriendStatus status) {
    Q_ASSERT(status < StatusImages.size() && "no image for status");
    return StatusImages[status];
}
