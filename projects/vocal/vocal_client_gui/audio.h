#pragma once

#include <QObject>
#include <memory>
#include <QtMultimedia/QtMultimedia>
#include <utils/buffer.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <mutex>


class TVocaGuiApp;
class TAudio: public QIODevice {
    Q_OBJECT
public:
    TAudio(TVocaGuiApp* app);
    ~TAudio();
public slots:
    void OnCallStarted();
    void OnCallFinished();
public:
    bool open(OpenMode mode) final;
    void close() final;
    qint64 writeData(const char *data, qint64 len) final;
    qint64 readData(char *data, qint64 maxlen) final;
    bool isSequential() const final;
    void OnDataReceived(TBuffer data);
private:
    QAudioFormat AudioFormat;
    QAudioDeviceInfo InputAudioDevice;
    QAudioDeviceInfo OutputAudioDevice;
    TVocaGuiApp* App;
    NVocal::TAudioQueue AudioQueue;
    std::mutex Lock;
    std::unique_ptr<QAudioInput> AudioInput;
    std::unique_ptr<QAudioOutput> AudioOutput;
};
