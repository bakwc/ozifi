#include "audio.h"


TAudio::TAudio()
    : InputAudioDevice(QAudioDeviceInfo::availableDevices(QAudio::AudioInput)[1])
    , OutputDeviceInfo(QAudioDeviceInfo::defaultOutputDevice())
{
    qDebug() << "IO Devices:" << InputAudioDevice.deviceName() << OutputDeviceInfo.deviceName();

    AudioFormat.setSampleRate(32000); //set frequency to 8000
    AudioFormat.setChannelCount(1); //set channels to mono
    AudioFormat.setSampleSize(16); //set sample sze to 16 bit
    AudioFormat.setSampleType(QAudioFormat::UnSignedInt ); //Sample type as usigned integer sample
    AudioFormat.setByteOrder(QAudioFormat::LittleEndian); //Byte order
    AudioFormat.setCodec("audio/pcm"); //set codec as simple audio/pcm
    if (!InputAudioDevice.isFormatSupported(AudioFormat)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        AudioFormat = InputAudioDevice.nearestFormat(AudioFormat);
    }
    AudioInput.reset(new QAudioInput(InputAudioDevice,AudioFormat));
    AudioOutput.reset(new QAudioOutput(OutputDeviceInfo, AudioFormat));


    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [this] {
        qDebug() << "recorded\n";
        AudioInput->stop();
//        qDebug() << Buffer.pos();
//        Buffer.seek(0);
//        qDebug() << Buffer.pos();
        AudioOutput->start(this);
    });
    qDebug() << "started";
    timer->start(5000);
//    Buffer.open(QIODevice::ReadWrite);
    this->open(QIODevice::ReadWrite);
    AudioInput->start(this);
}

TAudio::~TAudio() {
}

bool TAudio::open(QIODevice::OpenMode mode) {
    setOpenMode(mode);
    return true;
}

void TAudio::close() {
}

qint64 TAudio::writeData(const char *data, qint64 len) {
    qDebug() << "write" << len;
    return len;
}

qint64 TAudio::readData(char *data, qint64 maxlen) {
    qDebug() << "read" << maxlen;
    return -1;
}

bool TAudio::isSequential() const {
    return true;
}
