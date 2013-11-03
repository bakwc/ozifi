#pragma once

#include <QWidget>
#include <QPointer>
#include <QLineEdit>
#include <QPushButton>
#include <QImage>

#include <utils/buffer.h>

#include <projects/vocal/vocal_lib/defines.h>

#include <memory>

const size_t LOGIN_WINDOW_WIDTH = 300;
const size_t LOGIN_WINDOW_HEIGHT_NOCAPTCHA = 130;
const size_t LOGIN_WINDOW_HEIGHT_CAPTCHA = LOGIN_WINDOW_HEIGHT_NOCAPTCHA +
                                          NVocal::CAPTCHA_HEIGHT;

class TLoginWindow: public QWidget {
    Q_OBJECT
public:
    TLoginWindow();
    ~TLoginWindow();
signals:
    void Register(const QString& login);
    void Login(const QString& login);
    void DoLogin(const QString& captcha,
                 const QString& password);
    void DoRegister(const QString& captcha,
                    const QString& password,
                    const QString& email);
public slots:
    void OnBadLogin();
    void OnCaptchaAvailable(QImage image);
    void OnRegistrationFailed(const QString& message);
    void OnLoginFailed(const QString& message);
private slots:
    void OnRegisterButtonClicked();
    void OnLoginButtonClicked();
private:
    void paintEvent(QPaintEvent*);
private:
    QPointer<QWidget> CaptchaImageWidget;
    QImage CaptchaImage;
    QPointer<QLineEdit> LoginEdit;
    QPointer<QLineEdit> PasswordEdit;
    QPointer<QLineEdit> CaptchaEdit;
    QPointer<QPushButton> LoginButton;
    QPointer<QPushButton> RegisterButton;
};
