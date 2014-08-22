#include "audio.h"
#include "application.h"


TAudio::TAudio(TVocaGuiApp* app)
    : App(app)
{
    auto&& inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    auto&& outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

    for (size_t i = 0; i < inputDevices.size(); ++i) {
        qDebug() << "input audio device #" << i << " :" << inputDevices[i].deviceName();
    }
    for (size_t i = 0; i < outputDevices.size(); ++i) {
        qDebug() << "output audio device #" << i << ":" << outputDevices[i].deviceName();
    }

    if (inputDevices.empty() || outputDevices.empty()) {
        throw UException("missing audio device");
    }

    size_t audioInputDevice = 0;
    size_t audioOutputDevice = 0;
    try {
        audioInputDevice = App->Settings.GetParameter("audio_input_device");
    } catch (const UException& e) {
        App->Settings.SetParameter("audio_input_device", "0");
    }
    try {
        audioOutputDevice = App->Settings.GetParameter("audio_output_device");
    } catch (const UException&) {
        App->Settings.SetParameter("audio_output_device", "0");
    }
    if (audioInputDevice >= inputDevices.size()) {
        audioInputDevice = 0;
        App->Settings.SetParameter("audio_input_device", "0");
    }
    if (audioOutputDevice >= outputDevices.size()) {
        audioOutputDevice = 0;
        App->Settings.SetParameter("audio_output_device", "0");
    }

    InputAudioDevice = inputDevices[audioInputDevice];
    OutputAudioDevice = outputDevices[audioOutputDevice];

    qDebug() << "input audio device selected  #" << audioInputDevice << InputAudioDevice.deviceName();
    qDebug() << "output audio device selected #" << audioOutputDevice << OutputAudioDevice.deviceName();

    AudioFormat.setSampleRate(32000); //set frequency to 8000
    AudioFormat.setChannelCount(1); //set channels to mono
    AudioFormat.setSampleSize(16); //set sample sze to 16 bit
    AudioFormat.setSampleType(QAudioFormat::UnSignedInt ); //Sample type as usigned integer sample
    AudioFormat.setByteOrder(QAudioFormat::LittleEndian); //Byte order
    AudioFormat.setCodec("audio/pcm"); //set codec as simple audio/pcm
    if (!InputAudioDevice.isFormatSupported(AudioFormat)) {
        throw UException("audio device does not support required format");
    }
    AudioInput.reset(new QAudioInput(InputAudioDevice,AudioFormat));
    AudioInput->setNotifyInterval(1);
    AudioOutput.reset(new QAudioOutput(OutputAudioDevice, AudioFormat));
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
