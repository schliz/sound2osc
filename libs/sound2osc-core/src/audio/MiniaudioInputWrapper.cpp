// SPDX-License-Identifier: MIT
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>

#define MINIAUDIO_IMPLEMENTATION

// Suppress warnings from miniaudio
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// Allow miniaudio to use system headers
#include <sound2osc/audio/MiniaudioInputWrapper.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <sound2osc/logging/Logger.h>
#include <QDebug>

static void data_callback_c(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pOutput; // Unused
    MiniaudioInputWrapper* wrapper = static_cast<MiniaudioInputWrapper*>(pDevice->pUserData);
    if (wrapper) {
        wrapper->onData(pInput, frameCount);
    }
}

MiniaudioInputWrapper::MiniaudioInputWrapper(MonoAudioBuffer* buffer)
    : AudioInputInterface(buffer)
{
    initContext();
}

MiniaudioInputWrapper::~MiniaudioInputWrapper()
{
    stop();
    if (m_deviceInit) {
        ma_device_uninit(&m_device);
        m_deviceInit = false;
    }
    if (m_contextInit) {
        ma_context_uninit(&m_context);
        m_contextInit = false;
    }
}

void MiniaudioInputWrapper::initContext()
{
    if (ma_context_init(NULL, 0, NULL, &m_context) != MA_SUCCESS) {
        sound2osc::Logger::error("Failed to initialize miniaudio context");
        m_contextInit = false;
        return;
    }
    m_contextInit = true;
}

void MiniaudioInputWrapper::start()
{
    if (!m_deviceInit) {
        // Init default if not set
        initDevice(NULL);
    }
    
    if (m_deviceInit) {
        if (ma_device_start(&m_device) != MA_SUCCESS) {
            sound2osc::Logger::error("Failed to start miniaudio device");
        }
    }
}

void MiniaudioInputWrapper::stop()
{
    if (m_deviceInit) {
        ma_device_stop(&m_device);
    }
}

QStringList MiniaudioInputWrapper::getAvailableInputs() const
{
    QStringList names;
    if (!m_contextInit) return names;

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;

    if (ma_context_get_devices(const_cast<ma_context*>(&m_context), &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) == MA_SUCCESS) {
        for (ma_uint32 i = 0; i < captureCount; ++i) {
            names.append(QString::fromLocal8Bit(pCaptureInfos[i].name));
        }
    }
    return names;
}

QString MiniaudioInputWrapper::getDefaultInputName() const
{
    if (!m_contextInit) return QString();
    
    // miniaudio puts default first usually, or we can query it, but simpler to just return first or explicit default.
    // For now, let's just return the first one if available.
    QStringList inputs = getAvailableInputs();
    if (!inputs.isEmpty()) return inputs.first();
    return QString();
}

QString MiniaudioInputWrapper::getActiveInputName() const
{
    return m_activeInputName;
}

void MiniaudioInputWrapper::setInputByName(const QString& name)
{
    if (!m_contextInit) return;

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    
    ma_device_id* selectedId = NULL;
    ma_device_id id;

    if (ma_context_get_devices(&m_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) == MA_SUCCESS) {
        for (ma_uint32 i = 0; i < captureCount; ++i) {
            if (QString::fromLocal8Bit(pCaptureInfos[i].name) == name) {
                id = pCaptureInfos[i].id;
                selectedId = &id;
                break;
            }
        }
    }
    
    // Re-init device with new ID
    stop();
    if (m_deviceInit) {
        ma_device_uninit(&m_device);
        m_deviceInit = false;
    }
    
    m_activeInputName = name;
    initDevice(selectedId);
    start();
}

void MiniaudioInputWrapper::initDevice(const ma_device_id* id)
{
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.pDeviceID = const_cast<ma_device_id*>(id);
    config.capture.format = ma_format_f32;
    config.capture.channels = 2; // Stereo
    config.sampleRate = 44100;
    config.dataCallback = data_callback_c;
    config.pUserData = this;

    if (ma_device_init(&m_context, &config, &m_device) != MA_SUCCESS) {
        sound2osc::Logger::error("Failed to initialize miniaudio device");
        return;
    }
    m_deviceInit = true;
}

qreal MiniaudioInputWrapper::getVolume() const
{
    return m_volume;
}

void MiniaudioInputWrapper::setVolume(const qreal& value)
{
    m_volume = value;
    if (m_deviceInit) {
        ma_device_set_master_volume(&m_device, static_cast<float>(value));
    }
}

void MiniaudioInputWrapper::onData(const void* pInput, ma_uint32 frameCount)
{
    if (frameCount == 0) return;

    const float* samples = static_cast<const float*>(pInput);
    QVector<qreal> buffer(frameCount); // qreal is double by default in Qt, or float?
    // In sound2osc, MonoAudioBuffer expects QVector<qreal>. 
    // Miniaudio gives interleaved stereo float (L R L R).
    // But MonoAudioBuffer::putSamples takes QVector<qreal>& data, channelCount.
    // It handles conversion to mono.
    // So we just need to pass the raw data?
    // putSamples expects: "Converts PCM data with multiple channels to mono... data object will be resized"
    // Wait, putSamples implementation:
    // void MonoAudioBuffer::putSamples(QVector<qreal>& data, const int& channelCount)
    // It calls convertToMonoInplace.
    
    // So we need to put the interleaved data into QVector<qreal>.
    // samples has 2 * frameCount floats.
    
    int channels = 2;
    buffer.resize(frameCount * channels);
    for (ma_uint32 i = 0; i < frameCount * channels; ++i) {
        buffer[i] = static_cast<qreal>(samples[i]);
    }

    m_buffer->putSamples(buffer, channels);

    if (m_callback) {
        m_callback(frameCount);
    }
}
