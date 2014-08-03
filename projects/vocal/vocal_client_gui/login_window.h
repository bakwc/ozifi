#pragma once

#include <QWidget>
#include <QPointer>
#include <QLineEdit>
#include <QPushButton>
#include <QImage>
#include <QLabel>
#include <QMovie>

#include <utils/buffer.h>

#include <projects/vocal/vocal_lib/defines.h>

#include <memory>

const size_t LOGIN_WINDOW_WIDTH = 280;
const size_t LOGIN_WINDOW_HEIGHT_NOCAPTCHA = 130;
const size_t LOGIN_WINDOW_HEIGHT_CAPTCHA = LOGIN_WINDOW_HEIGHT_NOCAPTCHA +
                                          NVocal::CAPTCHA_HEIGHT;

enum ELoginScreen {
    LS_None,
    LS_Base,
    LS_Waiting,
    LS_Captcha
};

class TLoginWindow: public QWidget {
    Q_OBJECT
public:
    explicit TLoginWindow();
    ~TLoginWindow();
signals:
    void Register(const QString& login);
    void Login(const QString& login);
    void DoLogin(const QString& captcha,
                 const QString& password);
    void DoRegister(const QString& captcha,
                    const QString& password,
                    const QString& email);
    void Connect();
public slots:
    void OnCaptchaAvailable(QImage image);
    void OnRegistrationFailed(const QString& message);
    void OnLoginFailed(const QString& message);
private slots:
    void OnRegisterButtonClicked();
    void OnLoginButtonClicked();
    void OnContinueButtonClicked();
private:
    void paintEvent(QPaintEvent*);
    void SetScreen(ELoginScreen screen);
    void HideGuiElements();
    void ShowBaseScreen();
    void ShowWaitingScreen();
    void ShowCaptchaScreen();
private:
    ELoginScreen Screen;
    bool IsRegistering;
    QPointer<QWidget> CaptchaImageWidget;
    QImage CaptchaImage;
    QPointer<QLabel> ErrorLabel;
    QPointer<QLabel> CaptchaLabel;
    QPointer<QLabel> LoginLabel;
    QPointer<QLabel> PasswordLabel;
    QPointer<QLineEdit> LoginEdit;
    QPointer<QLineEdit> PasswordEdit;
    QPointer<QLineEdit> CaptchaEdit;
    QPointer<QPushButton> LoginButton;
    QPointer<QPushButton> RegisterButton;
    QPointer<QPushButton> ContinueButton;
    QPointer<QMovie> WaitAnimation;
    QPointer<QLabel> WaitLabel;
    QString ErrorMessage;
};
