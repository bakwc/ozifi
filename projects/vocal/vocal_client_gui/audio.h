#pragma once

#include <QObject>
#include <memory>
#include <deque>
#include <QtMultimedia/QtMultimedia>
#include <utils/buffer.h>
#include <mutex>

class TAudioQueue {
      std::deque<char> Data;
public:
    void Add(TBuffer buff) {
        Data.insert(Data.end(), buff.Data(), buff.Data() + buff.Size());
    }

    void Get(char *array, size_t bytesToRead) {
        if (Data.size() < bytesToRead) {
            Data.insert(Data.end(), bytesToRead - Data.size(), 0);
        }
        std::copy(Data.begin(), Data.begin() + bytesToRead, array);
        Data.erase(Data.begin(), Data.begin()+bytesToRead);
    }
};

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
    QAudioDeviceInfo OutputDeviceInfo;
    TVocaGuiApp* App;
    TAudioQueue AudioQueue;
    std::mutex Lock;
    std::unique_ptr<QAudioInput> AudioInput;
    std::unique_ptr<QAudioOutput> AudioOutput;
};
