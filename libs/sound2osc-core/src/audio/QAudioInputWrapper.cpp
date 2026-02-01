// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <sound2osc/audio/QAudioInputWrapper.h>

#include <sound2osc/core/utils.h>

#include <QList>
#include <QVector>
#include <QByteArray>
#include <QDebug>

#include <iostream>

QAudioInputWrapper::QAudioInputWrapper(MonoAudioBuffer *buffer)
    : AudioInputInterface(buffer)
    , m_audioSource(nullptr)
    , m_audioIODevice(nullptr)
{
    // Set up the desired format:
    // If the input device doesn't support this the nearest format will be used.
    m_desiredAudioFormat.setSampleRate(44100);
    m_desiredAudioFormat.setChannelCount(2);
    // Qt6: Use setSampleFormat instead of setSampleSize/setSampleType/setCodec/setByteOrder
    m_desiredAudioFormat.setSampleFormat(QAudioFormat::Int16);
}

QAudioInputWrapper::~QAudioInputWrapper()
{
    // Close and delete previous input device:
    if (m_audioSource) {
        m_audioSource->stop();
    }
    if (m_audioIODevice && m_audioIODevice->isOpen()) {
        m_audioIODevice->close();
    }
    delete m_audioSource;
}

QStringList QAudioInputWrapper::getAvailableInputs() const
{
    // get List of input device names from QList<QAudioDevice>:
    QStringList deviceList;
    const QList<QAudioDevice> devices = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : devices) {
        deviceList.append(device.description());
    }
    return deviceList;
}

QString QAudioInputWrapper::getDefaultInputName() const
{
    QStringList devices = getAvailableInputs();
    // if there are no devices, return empty string:
    if (devices.size() <= 0) return QString();
    QAudioDevice defaultDevice = QMediaDevices::defaultAudioInput();
    return defaultDevice.description();
}

void QAudioInputWrapper::setInputByName(const QString &inputName)
{
    // Close and delete previous input device:
    if (m_audioSource) {
        m_audioSource->stop();
    }
    if (m_audioIODevice && m_audioIODevice->isOpen()) {
        disconnect(m_audioIODevice, &QIODevice::readyRead, this, &QAudioInputWrapper::audioDataReady);
        m_audioIODevice->close();
    }
    delete m_audioSource;
    m_audioSource = nullptr;
    m_audioIODevice = nullptr;

    // Get device info of new input:
    const QList<QAudioDevice> devices = QMediaDevices::audioInputs();
    QAudioDevice selectedDevice = QMediaDevices::defaultAudioInput();
    
    for (const QAudioDevice &device : devices) {
        if (device.description() == inputName) {
            selectedDevice = device;
            break;
        }
    }

    // check if desired format is supported:
    if (!selectedDevice.isFormatSupported(m_desiredAudioFormat)) {
        qWarning() << "Default audio format not supported, trying to use the nearest.";
        // Qt6: No nearestFormat(), we need to pick a supported format
        // Try to get a supported format from the device's preferred format
        m_actualAudioFormat = selectedDevice.preferredFormat();
        // Override with our preferences where possible
        m_actualAudioFormat.setSampleRate(44100);
        m_actualAudioFormat.setChannelCount(2);
        if (!selectedDevice.isFormatSupported(m_actualAudioFormat)) {
            // Fall back to device's preferred format entirely
            m_actualAudioFormat = selectedDevice.preferredFormat();
        }
    } else {
        m_actualAudioFormat = m_desiredAudioFormat;
    }

    // create new input:
    m_activeInputName = inputName;
    m_audioSource = new QAudioSource(selectedDevice, m_actualAudioFormat, this);
    m_audioSource->setVolume(1.0f);
    m_audioIODevice = m_audioSource->start();
    if (m_audioIODevice) {
        connect(m_audioIODevice, &QIODevice::readyRead, this, &QAudioInputWrapper::audioDataReady);
    }
}

void QAudioInputWrapper::start()
{
    if (m_audioSource) {
        if (m_audioSource->state() == QAudio::StoppedState) {
             m_audioIODevice = m_audioSource->start();
             if (m_audioIODevice) {
                connect(m_audioIODevice, &QIODevice::readyRead, this, &QAudioInputWrapper::audioDataReady);
             }
        }
    } else {
        // Try starting default
        setInputByName(getDefaultInputName());
    }
}

void QAudioInputWrapper::stop()
{
    if (m_audioSource) {
        m_audioSource->stop();
    }
}

qreal QAudioInputWrapper::getVolume() const
{
    if (!m_audioSource) return 0.0;
    return m_audioSource->volume();
}

void QAudioInputWrapper::setVolume(const qreal &value)
{
    if (!m_audioSource) return;
    m_audioSource->setVolume(static_cast<float>(limit(0.0, value, 1.0)));
}

void QAudioInputWrapper::audioDataReady()
{
    if (!m_audioIODevice) return;
    
    // read data from input as QByteArray:
    QByteArray data = m_audioIODevice->readAll();
    
    // Qt6: bytesPerSample() returns the size per sample
    const int bytesPerSample = m_actualAudioFormat.bytesPerSample();
    if (bytesPerSample <= 0) return;
    
    const qsizetype numSamples = data.size() / bytesPerSample;
    if (numSamples <= 0) return;
    
    QVector<qreal> realData(numSamples);
    const char *ptr = data.constData();

    // Handle different sample formats
    QAudioFormat::SampleFormat sampleFormat = m_actualAudioFormat.sampleFormat();
    
    for (qsizetype i = 0; i < numSamples; ++i) {
        qreal scaled = 0.0;
        
        switch (sampleFormat) {
        case QAudioFormat::Int16:
            {
                const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
                scaled = static_cast<qreal>(pcmSample) / 32768.0;
            }
            break;
        case QAudioFormat::Int32:
            {
                const qint32 pcmSample = *reinterpret_cast<const qint32*>(ptr);
                scaled = static_cast<qreal>(pcmSample) / 2147483648.0;
            }
            break;
        case QAudioFormat::Float:
            {
                const float pcmSample = *reinterpret_cast<const float*>(ptr);
                scaled = static_cast<qreal>(pcmSample);
            }
            break;
        case QAudioFormat::UInt8:
            {
                const quint8 pcmSample = *reinterpret_cast<const quint8*>(ptr);
                scaled = (static_cast<qreal>(pcmSample) - 128.0) / 128.0;
            }
            break;
        default:
            // Unknown format, skip
            break;
        }
        
        realData[i] = scaled;
        ptr += bytesPerSample;
    }

    // Call MonoAudioBuffer as next element in processing chain:
    m_buffer->putSamples(realData, m_actualAudioFormat.channelCount());

    if (m_callback) {
        m_callback(realData.size());
    }
}
