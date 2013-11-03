#include <QDesktopWidget>
#include <QDebug>
#include <QLabel>
#include <QGridLayout>
#include <QImage>
#include <QPainter>

#include "login_window.h"

TLoginWindow::TLoginWindow() {
    qDebug() << Q_FUNC_INFO;
    this->setFixedSize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT_NOCAPTCHA);
    this->setGeometry(QDesktopWidget().availableGeometry().center().x() - (this->width() / 2),
                      QDesktopWidget().availableGeometry().center().y() - (this->height() / 2),
                       this->width(), this->height());

    QGridLayout* layout = new QGridLayout(this);

    QLabel* loginLabel = new QLabel(tr("Login:"), this);
    layout->addWidget(loginLabel, 0, 0);

    QLabel* passwordLabel = new QLabel(tr("Password:"), this);
    layout->addWidget(passwordLabel, 1, 0);

    CaptchaImageWidget = new QWidget();

    CaptchaImageWidget->setFixedSize(0, 0);
    layout->addWidget(CaptchaImageWidget, 2, 0, 1, 2);

    QLabel* captchaLabel = new QLabel(tr("Captcha:"), this);
    layout->addWidget(captchaLabel, 3, 0);

    LoginEdit = new QLineEdit(this);
    layout->addWidget(LoginEdit, 0, 1);

    PasswordEdit = new QLineEdit(this);
    layout->addWidget(PasswordEdit, 1, 1);

    CaptchaEdit = new QLineEdit(this);
    layout->addWidget(CaptchaEdit, 3, 1);
    CaptchaEdit->setEnabled(false);

    LoginButton = new QPushButton(tr("Login"), this);
    connect(LoginButton.data(), &QPushButton::clicked, this,
            &TLoginWindow::OnLoginButtonClicked);
    layout->addWidget(LoginButton, 4, 0);

    RegisterButton = new QPushButton(tr("Register"), this);
    connect(RegisterButton.data(), &QPushButton::clicked, this,
            &TLoginWindow::OnRegisterButtonClicked);
    layout->addWidget(RegisterButton, 4, 1);


    layout->setColumnMinimumWidth(0, 120);
    layout->setColumnMinimumWidth(1, 120);

    this->show();
}

TLoginWindow::~TLoginWindow() {
}

void TLoginWindow::OnRegisterButtonClicked() {
    qDebug() << Q_FUNC_INFO;
    if (CaptchaEdit->text().size() != 0 &&
        LoginEdit->text().size() != 0 &&
        PasswordEdit->text().size() != 0)
    {
        emit DoRegister(CaptchaEdit->text(),
                        PasswordEdit->text(),
                        "");
        // todo: use email
    } if (LoginEdit->text().size() != 0) {
        emit Register(LoginEdit->text());
    }
}

void TLoginWindow::OnLoginButtonClicked() {
    qDebug() << Q_FUNC_INFO;
    if (CaptchaEdit->text().size() != 0 &&
        LoginEdit->text().size() != 0 &&
        PasswordEdit->text().size() != 0)
    {
        emit DoLogin(CaptchaEdit->text(), PasswordEdit->text());
    } if (LoginEdit->text().size() != 0) {
        emit Login(LoginEdit->text());
    }
}

void TLoginWindow::paintEvent(QPaintEvent*) {
    if (CaptchaImage.width() == 0) {
        return;
    }
    QPainter painter(this);
    painter.drawImage(CaptchaImageWidget->x(), CaptchaImageWidget->y(), CaptchaImage);
}

void TLoginWindow::OnBadLogin() {
    qDebug() << Q_FUNC_INFO;
    // todo: show message about bad login
    LoginEdit->setText("");
}

void TLoginWindow::OnCaptchaAvailable(QImage image) {
    qDebug() << Q_FUNC_INFO;
    this->setFixedSize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT_CAPTCHA);
    CaptchaImageWidget->setFixedSize(NVocal::CAPTCHA_WIDTH, NVocal::CAPTCHA_HEIGHT);
    CaptchaImage = image;
    CaptchaEdit->setEnabled(true);
    this->update();
}

void TLoginWindow::OnRegistrationFailed(const QString& message) {
    qDebug() << Q_FUNC_INFO;
    qDebug() << message;
    // todo: show qmessage
}

void TLoginWindow::OnLoginFailed(const QString &message) {
    qDebug() << Q_FUNC_INFO;
    qDebug() << message;
    // todo: show qmessage
}
