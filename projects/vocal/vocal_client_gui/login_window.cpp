#include <QDesktopWidget>
#include <QDebug>
#include <QGridLayout>
#include <QImage>
#include <QPainter>

#include "login_window.h"

TLoginWindow::TLoginWindow()
    : QWidget(NULL)
    , Screen(LS_None)
{
    this->setFixedSize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT_NOCAPTCHA);
    this->setGeometry(QDesktopWidget().availableGeometry().center().x() - (this->width() / 2),
                      QDesktopWidget().availableGeometry().center().y() - (this->height() / 2),
                       this->width(), this->height());

    ErrorLabel = new QLabel(this);
    ErrorLabel->setStyleSheet("QLabel { color: red; }");

    LoginLabel = new QLabel(tr("Login:"), this);
    PasswordLabel = new QLabel(tr("Password:"), this);

    CaptchaImageWidget = new QWidget(this);

    CaptchaImageWidget->setFixedSize(0, 0);

    CaptchaLabel = new QLabel(tr("Captcha:"), this);

    LoginEdit = new QLineEdit(this);
    PasswordEdit = new QLineEdit(this);

    CaptchaEdit = new QLineEdit(this);

    WaitAnimation = new QMovie(":/icons/ajax-loader.gif", QByteArray(), this);
    WaitLabel = new QLabel(this);
    WaitLabel->setMovie(WaitAnimation);

    LoginButton = new QPushButton(tr("Login"), this);
    connect(LoginButton.data(), &QPushButton::clicked, this,
            &TLoginWindow::OnLoginButtonClicked);

    RegisterButton = new QPushButton(tr("Register"), this);
    connect(RegisterButton.data(), &QPushButton::clicked, this,
            &TLoginWindow::OnRegisterButtonClicked);

    ContinueButton = new QPushButton(tr("Continue"), this);
    connect(ContinueButton.data(), &QPushButton::clicked, this,
            &TLoginWindow::OnContinueButtonClicked);

    SetScreen(LS_Base);

    this->show();
}

TLoginWindow::~TLoginWindow() {
}

void TLoginWindow::OnRegisterButtonClicked() {
    SetScreen(LS_Waiting);
    IsRegistering = true;
    emit Register(LoginEdit->text());
}

void TLoginWindow::OnLoginButtonClicked() {
    SetScreen(LS_Waiting);
    IsRegistering = false;
    emit Login(LoginEdit->text());
}

void TLoginWindow::OnContinueButtonClicked() {
    SetScreen(LS_Waiting);
    CaptchaImage = QImage();
    if (IsRegistering) {
        emit DoRegister(CaptchaEdit->text(),
                        PasswordEdit->text(),
                        "");
    } else {
        emit DoLogin(CaptchaEdit->text(), PasswordEdit->text());
    }
}

void TLoginWindow::paintEvent(QPaintEvent*) {
    if (CaptchaImage.width() == 0) {
        return;
    }
    QPainter painter(this);
    painter.drawImage(CaptchaImageWidget->x(), CaptchaImageWidget->y(), CaptchaImage);
}

void TLoginWindow::SetScreen(ELoginScreen screen) {
    if (Screen == screen) {
        return;
    }
    HideGuiElements();
    Screen = screen;
    switch (Screen) {
    case LS_Base:
        ShowBaseScreen();
        break;
    case LS_Waiting:
        ShowWaitingScreen();
        break;
    case LS_Captcha:
        ShowCaptchaScreen();
        break;
    default:
        break;
    }
    ErrorMessage.clear();
}

void TLoginWindow::HideGuiElements() {
    CaptchaLabel->hide();
    LoginLabel->hide();
    PasswordLabel->hide();
    LoginEdit->hide();
    PasswordEdit->hide();
    CaptchaEdit->hide();
    LoginButton->hide();
    RegisterButton->hide();
    WaitLabel->hide();
    ErrorLabel->hide();
    ContinueButton->hide();
    this->update();
}

void TLoginWindow::ShowBaseScreen() {
    int currentOffset = 0;
    currentOffset += 20;

    if (ErrorMessage.size() != 0) {
        ErrorLabel->setText(ErrorMessage);
        ErrorLabel->move(LOGIN_WINDOW_WIDTH / 2 - ErrorLabel->width() / 2, currentOffset);
        ErrorLabel->show();
        currentOffset += ErrorLabel->height() + 10;
    }

    LoginLabel->move(20, currentOffset + 5);
    LoginLabel->show();
    LoginEdit->move(LOGIN_WINDOW_WIDTH / 3, currentOffset);
    LoginEdit->setFixedWidth(LOGIN_WINDOW_WIDTH - LoginEdit->x() - 20);
    LoginEdit->show();
    currentOffset += LoginEdit->height() + 10;

    PasswordLabel->move(20, currentOffset + 5);
    PasswordLabel->show();
    PasswordEdit->move(LOGIN_WINDOW_WIDTH / 3, currentOffset);
    PasswordEdit->setFixedWidth(LOGIN_WINDOW_WIDTH - LoginEdit->x() - 20);
    PasswordEdit->show();
    currentOffset += PasswordEdit->height() + 10;

    LoginButton->move(LOGIN_WINDOW_WIDTH / 4 - LoginButton->width() / 2, currentOffset);
    LoginButton->show();
    RegisterButton->move(LOGIN_WINDOW_WIDTH * 3 / 4 - RegisterButton->width() / 2, currentOffset);
    RegisterButton->show();
    currentOffset += RegisterButton->height() + 10;

    this->setFixedHeight(currentOffset);
    this->update();
}

void TLoginWindow::ShowWaitingScreen() {
    WaitAnimation->start();
    WaitLabel->move(this->width() / 2 - WaitAnimation->currentImage().width() / 2,
                    this->height() / 2 - WaitAnimation->currentImage().height() / 2);
    WaitLabel->show();
    this->update();
}

void TLoginWindow::ShowCaptchaScreen() {
    CaptchaImageWidget->setFixedSize(CaptchaImage.width(), CaptchaImage.height());
    int currentOffset = 0;
    currentOffset += 20;

    int minWidth = CaptchaImageWidget->width() + 40;
    if (this->width() < minWidth) {
        this->setFixedWidth(minWidth);
    }
    CaptchaImageWidget->move(this->width() / 2 - CaptchaImageWidget->width() / 2, currentOffset);
    currentOffset += CaptchaImage.height() + 10;

    CaptchaLabel->move(20, currentOffset + 5);
    CaptchaLabel->show();
    CaptchaEdit->move(this->width() / 3, currentOffset);
    CaptchaEdit->show();
    currentOffset += CaptchaEdit->height() + 10;

    ContinueButton->move(this->width() / 2 - ContinueButton->width() / 2, currentOffset);
    ContinueButton->show();
    currentOffset += ContinueButton->height() + 10;

    this->setFixedHeight(currentOffset);
    this->update();
}

void TLoginWindow::OnCaptchaAvailable(QImage image) {
    CaptchaImage = image;
    SetScreen(LS_Captcha);
}

void TLoginWindow::OnRegistrationFailed(const QString& message) {
    ErrorMessage = message;
    SetScreen(LS_Base);
}

void TLoginWindow::OnLoginFailed(const QString &message) {
    ErrorMessage = message;
    SetScreen(LS_Base);
}
