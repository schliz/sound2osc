// SPDX-License-Identifier: MIT
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>

#ifndef MINIAUDIOINPUTWRAPPER_H
#define MINIAUDIOINPUTWRAPPER_H

#include <sound2osc/audio/AudioInputInterface.h>
#include <sound2osc/audio/MonoAudioBuffer.h>

#include <QString>
#include <QVector>

// Forward declaration to avoid including miniaudio in header if possible, 
// but we need struct definitions for members unless we use PIMPL.
// For simplicity, we include it but do NOT define implementation.
#include "miniaudio.h"

class MiniaudioInputWrapper : public AudioInputInterface
{
public:
    explicit MiniaudioInputWrapper(MonoAudioBuffer* buffer);
    ~MiniaudioInputWrapper() override;

    void start() override;
    void stop() override;
    void setCallback(Callback callback) override { m_callback = callback; }

    QStringList getAvailableInputs() const override;
    QString getDefaultInputName() const override;
    QString getActiveInputName() const override;
    void setInputByName(const QString& name) override;

    qreal getVolume() const override;
    void setVolume(const qreal& value) override;

    // Public method to handle data, called by static C callback
    void onData(const void* pInput, ma_uint32 frameCount);

private:
    ma_context m_context;
    ma_device m_device;
    bool m_contextInit{false};
    bool m_deviceInit{false};
    
    QString m_activeInputName;
    qreal m_volume{1.0};
    Callback m_callback;
    
    void initContext();
    void initDevice(const ma_device_id* id);
};

#endif // MINIAUDIOINPUTWRAPPER_H
