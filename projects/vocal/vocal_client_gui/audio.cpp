#include "audio.h"
#include "application.h"


TAudio::TAudio(TVocaGuiApp* app)
    : InputAudioDevice(QAudioDeviceInfo::availableDevices(QAudio::AudioInput)[1])
    , OutputDeviceInfo(QAudioDeviceInfo::defaultOutputDevice())
    , App(app)
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
    AudioInput->setNotifyInterval(1);
    AudioOutput.reset(new QAudioOutput(OutputDeviceInfo, AudioFormat));
    this->open(QIODevice::ReadWrite);
}

TAudio::~TAudio() {
}

void TAudio::OnCallStarted() {
    AudioInput->start(this);
    AudioOutput->start(this);
}

void TAudio::OnCallFinished() {
    AudioInput->stop();
    AudioOutput->stop();
}

bool TAudio::open(QIODevice::OpenMode mode) {
    setOpenMode(mode);
    return true;
}

void TAudio::close() {
}

qint64 TAudio::writeData(const char *data, qint64 len) {
    App->Client->ProvideAudioData(TBuffer(data, len));
    return len;
}

qint64 TAudio::readData(char *data, qint64 maxlen) {
    if (maxlen == 0) {
        return 0;
    }
    maxlen = std::min((int)maxlen, 640);
    std::lock_guard<std::mutex> guard(Lock);
    AudioQueue.Get(data, maxlen);
    return maxlen;
}

bool TAudio::isSequential() const {
    return true;
}

void TAudio::OnDataReceived(TBuffer data) {
    std::lock_guard<std::mutex> guard(Lock);
    AudioQueue.Add(data);
}
