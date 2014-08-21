#pragma once

#include <QObject>
#include <memory>
#include <QtMultimedia/QtMultimedia>

class TAudio: public QIODevice {
    Q_OBJECT
public:
    TAudio();
    ~TAudio();
public:
    bool open(OpenMode mode) final;
    void close() final;
    qint64 writeData(const char *data, qint64 len) final;
    qint64 readData(char *data, qint64 maxlen) final;
    bool isSequential() const final;
private:
    QAudioFormat AudioFormat;
    QAudioDeviceInfo InputAudioDevice;
    QAudioDeviceInfo OutputDeviceInfo;
    std::unique_ptr<QAudioInput> AudioInput;
    std::unique_ptr<QAudioOutput> AudioOutput;
};
