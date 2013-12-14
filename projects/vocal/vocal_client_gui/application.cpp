#include <QDebug>

#include "application.h"

using namespace std;
using namespace std::placeholders;

TVocaGuiApp::TVocaGuiApp(int &argc, char** argv)
    : QApplication(argc, argv)
    , Status(ST_None)
{
    NVocal::TClientConfig config;
    config.CaptchaAvailableCallback = std::bind(&TVocaGuiApp::OnCaptcha, this, _1);
    config.RegisterResultCallback = std::bind(&TVocaGuiApp::OnRegistered, this, _1);
    config.LoginResultCallback = std::bind(&TVocaGuiApp::OnLogined, this, _1);
    config.ConnectedCallback = std::bind(&TVocaGuiApp::OnConnected, this, _1);
    config.OnFriendAdded = std::bind(&TVocaGuiApp::OnFriendAdded, this, _1);
    config.OnFriendRemoved = std::bind(&TVocaGuiApp::OnFriendRemoved, this, _1);
    config.OnFriendUpdated = std::bind(&TVocaGuiApp::OnFriendUpdated, this, _1);
    config.OnMessageReceived = std::bind(&TVocaGuiApp::OnMessageReceived, this, _1);
    config.StateDir = "data";
    connect(this, &TVocaGuiApp::RegistrationSuccess,
            this, &TVocaGuiApp::OnSuccesfullyRegistered);
    ChatWindows.reset(new TChatWindows());
    connect(this, &TVocaGuiApp::MessageReceived,
            ChatWindows.get(), &TChatWindows::ShowMessage);
    Client.reset(new NVocal::TClient(config));
    if (Client->HasConnectData()) {
        LaunchMain();
    } else {
        LaunchLogin();
    }
}

TVocaGuiApp::~TVocaGuiApp() {
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
    qDebug() << Q_FUNC_INFO;
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
    QString frndLogin = QString::fromStdString(message.From);
    QString messageText = QString::fromStdString(message.Text);
    emit MessageReceived(frndLogin, messageText);
}

void TVocaGuiApp::Register(const QString& login) {
    if (Status != ST_None) {
        return;
    }
    try {
        Status = ST_Registering;
        Client->Register(login.toStdString());
    } catch (const std::exception&) {
        Status = ST_None;
        emit BadLogin();
    }
}

void TVocaGuiApp::Login(const QString& login) {
    qDebug() << Q_FUNC_INFO;
    if (Status != ST_None) {
        return;
    }
    try {
        Status = ST_Logining;
        qDebug() << login;
        Client->Login(login.toStdString());
    } catch (const std::exception&) {
        Status = ST_None;
        emit BadLogin();
    }
}

void TVocaGuiApp::DoLogin(const QString& captcha, const QString& password) {
    Client->Login(password.toStdString(), captcha.toStdString());
    // todo: handle exceptions
}

void TVocaGuiApp::DoRegister(const QString& captcha,
                             const QString& password,
                             const QString& email)
{
    Client->Register(password.toStdString(),
                     email.toStdString(),
                     captcha.toStdString());
    // todo: handle exceptions
}

void TVocaGuiApp::LaunchLogin() {
    LoginWindow.reset(new TLoginWindow());
    connect(LoginWindow.get(), &TLoginWindow::Register,
            this, &TVocaGuiApp::Register);
    connect(LoginWindow.get(), &TLoginWindow::Login,
            this, &TVocaGuiApp::Login);
    connect(this, &TVocaGuiApp::BadLogin,
            LoginWindow.get(), &TLoginWindow::OnBadLogin);
    connect(this, &TVocaGuiApp::CaptchaAvailable,
            LoginWindow.get(), &TLoginWindow::OnCaptchaAvailable);
    connect(LoginWindow.get(), &TLoginWindow::DoLogin,
            this, &TVocaGuiApp::DoLogin);
    connect(LoginWindow.get(), &TLoginWindow::DoRegister,
            this, &TVocaGuiApp::DoRegister);
    connect(this, &TVocaGuiApp::RegistrationFailed,
            LoginWindow.get(), &TLoginWindow::OnRegistrationFailed);
}

void TVocaGuiApp::LaunchMain() {
    FriendListModel.reset(new TFriendListModel(*Client));
    MainWindow.reset(new TMainWindow(&ImageStorage, FriendListModel.get()));
    Client->Connect();
}

void TVocaGuiApp::OnSuccesfullyRegistered() {
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(Client->HasConnectData() && "no data to connect");
    LoginWindow.reset(nullptr);
    LaunchMain();
}

void TVocaGuiApp::OnFriendAdded(const NVocal::TFriendRef& frnd) {
    if (FriendListModel) {
        FriendListModel->OnFriendAdded(frnd);
    }
}

void TVocaGuiApp::OnFriendRemoved(const NVocal::TFriendRef& frnd) {
    if (FriendListModel) {
        FriendListModel->OnFriendRemoved(frnd);
    }
}

void TVocaGuiApp::OnFriendUpdated(const NVocal::TFriendRef& frnd) {
    if (FriendListModel) {
        FriendListModel->OnFriendUpdated(frnd);
    }
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
        case NVocal::FS_Available: frndData.Status = FS_Away; break;
        default: Q_ASSERT(!"unexpected status");
        }
        result.setValue(frndData);
    }
    return result;
}

void TFriendListModel::OnFriendAdded(const NVocal::TFriendRef& frnd) {
    qDebug() << Q_FUNC_INFO;
    beginInsertRows(QModelIndex(), Friends.size(), Friends.size());
    Friends.push_back(frnd);
    endInsertRows();
}

void TFriendListModel::OnFriendRemoved(const NVocal::TFriendRef& frnd) {
    qDebug() << Q_FUNC_INFO;
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

void TFriendListModel::OnFriendUpdated(const NVocal::TFriendRef& frnd) {
    qDebug() << Q_FUNC_INFO;
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
