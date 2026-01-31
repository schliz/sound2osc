// SPDX-License-Identifier: MIT
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>

#include <sound2osc/core/Sound2OscEngine.h>
#include <sound2osc/logging/Logger.h>
#include <sound2osc/dsp/FFTAnalyzer.h>
#include <QJsonArray>

namespace sound2osc {

Sound2OscEngine::Sound2OscEngine(std::shared_ptr<SettingsManager> settings, QObject *parent)
    : QObject(parent)
    , m_running(false)
    , m_lowSoloMode(false)
    , m_settings(std::move(settings))
{
    initializeComponents();
    connectComponents();
}

Sound2OscEngine::~Sound2OscEngine()
{
    stop();
}

void Sound2OscEngine::initializeComponents()
{
    Logger::info("Initializing Sound2Osc Engine...");

    // 1. Audio Buffer (4x NUM_SAMPLES for overlap/safety)
    // Using 4096 * 4 samples
    m_audioBuffer = std::make_unique<MonoAudioBuffer>(4096 * 4);

    // 2. Audio Input
    m_audioInput = std::make_unique<QAudioInputWrapper>(m_audioBuffer.get());

    // 3. OSC Manager
    m_osc = std::make_unique<OSCNetworkManager>();

    // 4. BPM Components
    m_bpmOsc = std::make_unique<BPMOscControler>(*m_osc);
    m_bpmDetector = std::make_unique<BPMDetector>(*m_audioBuffer, m_bpmOsc.get());

    // 5. Trigger Generators
    // We create them with unique_ptr for ownership, but keep raw pointers in vector for FFTAnalyzer
    m_bass = std::make_unique<TriggerGenerator>("bass", m_osc.get(), true, false, 80);
    m_loMid = std::make_unique<TriggerGenerator>("loMid", m_osc.get(), true, false, 400);
    m_hiMid = std::make_unique<TriggerGenerator>("hiMid", m_osc.get(), true, false, 1000);
    m_high = std::make_unique<TriggerGenerator>("high", m_osc.get(), true, false, 5000);
    m_envelope = std::make_unique<TriggerGenerator>("envelope", m_osc.get(), false);
    m_silence = std::make_unique<TriggerGenerator>("silence", m_osc.get(), false, true);

    m_triggerInterfaces.reserve(6);
    m_triggerInterfaces.append(m_bass.get());
    m_triggerInterfaces.append(m_loMid.get());
    m_triggerInterfaces.append(m_hiMid.get());
    m_triggerInterfaces.append(m_high.get());
    m_triggerInterfaces.append(m_envelope.get());
    m_triggerInterfaces.append(m_silence.get());

    // 6. FFT Analyzer
    m_fft = std::make_unique<FFTAnalyzer>(*m_audioBuffer, m_triggerInterfaces);
}

void Sound2OscEngine::connectComponents()
{
    // Configure timers
    // FFT and BPM run at ~44 Hz (approx 23ms)
    m_fftTimer.setInterval(1000 / 44);
    m_fftTimer.setSingleShot(false);
    connect(&m_fftTimer, &QTimer::timeout, this, &Sound2OscEngine::onFftTimer);

    m_bpmTimer.setInterval(1000 / 44);
    m_bpmTimer.setSingleShot(false);
    connect(&m_bpmTimer, &QTimer::timeout, this, &Sound2OscEngine::onBpmTimer);

    // Status timer runs every 5 seconds
    m_statusTimer.setInterval(5000);
    m_statusTimer.setSingleShot(false);
    connect(&m_statusTimer, &QTimer::timeout, this, &Sound2OscEngine::onStatusTimer);
}

void Sound2OscEngine::applySettings()
{
    if (!m_settings) return;

    // OSC Settings
    m_osc->setIpAddress(QHostAddress(m_settings->oscIpAddress()));
    m_osc->setUdpTxPort(m_settings->oscUdpTxPort());
    m_osc->setUdpRxPort(m_settings->oscUdpRxPort());
    m_osc->setTcpPort(m_settings->oscTcpPort());
    m_osc->setUseTcp(m_settings->useTcp());
    m_osc->setEnabled(m_settings->oscEnabled());

    // Audio Input
    QString inputDevice = m_settings->inputDeviceName();
    if (!inputDevice.isEmpty()) {
        m_audioInput->setInputByName(inputDevice);
    }
}

void Sound2OscEngine::start()
{
    if (m_running) return;
    
    Logger::info("Starting Engine...");
    applySettings();
    
    m_running = true;
    m_fftTimer.start();
    m_bpmTimer.start();
    m_statusTimer.start();
}

void Sound2OscEngine::stop()
{
    if (!m_running) return;
    
    Logger::info("Stopping Engine...");
    m_running = false;
    m_fftTimer.stop();
    m_bpmTimer.stop();
    m_statusTimer.stop();
}

void Sound2OscEngine::onFftTimer()
{
    if (!m_running) return;
    m_fft->calculateFFT(m_lowSoloMode);
}

void Sound2OscEngine::onBpmTimer()
{
    if (!m_running) return;
    m_bpmDetector->detectBPM();
}

void Sound2OscEngine::onStatusTimer()
{
    if (!m_running) return;
    
    // Only log if verbose is enabled in Logger (handled inside Logger)
    // But we construct the string anyway, so check logic might be good
    // For now, mirroring headless behavior
    float bpm = m_bpmDetector->getBPM();
    Logger::debug("Status: BPM=%1, Audio=%2", 
                  QString::number(static_cast<double>(bpm), 'f', 1),
                  m_audioInput->getActiveInputName());
}

void Sound2OscEngine::setLowSoloMode(bool enabled)
{
    m_lowSoloMode = enabled;
}

QJsonObject Sound2OscEngine::toState() const
{
    QJsonObject state;
    
    // Global Settings
    state["lowSoloMode"] = m_lowSoloMode;
    
    // DSP Settings
    QJsonObject dsp;
    dsp["gain"] = m_fft->getScaledSpectrum().getGain();
    dsp["compression"] = m_fft->getScaledSpectrum().getCompression();
    dsp["decibel"] = m_fft->getScaledSpectrum().getDecibelConversion();
    dsp["agc"] = m_fft->getScaledSpectrum().getAgcEnabled();
    state["dsp"] = dsp;
    
    // BPM Settings
    QJsonObject bpm;
    bpm["min"] = m_bpmDetector->getMinBPM();
    bpm["mute"] = m_bpmOsc->getBPMMute();
    bpm["osc"] = m_bpmOsc->toState();
    state["bpm"] = bpm;
    
    // Triggers
    QJsonObject triggers;
    triggers["bass"] = m_bass->toState();
    triggers["loMid"] = m_loMid->toState();
    triggers["hiMid"] = m_hiMid->toState();
    triggers["high"] = m_high->toState();
    triggers["envelope"] = m_envelope->toState();
    triggers["silence"] = m_silence->toState();
    state["triggers"] = triggers;
    
    return state;
}

void Sound2OscEngine::fromState(const QJsonObject& state)
{
    // Global Settings
    if (state.contains("lowSoloMode")) {
        m_lowSoloMode = state["lowSoloMode"].toBool();
    }
    
    // DSP Settings
    if (state.contains("dsp")) {
        QJsonObject dsp = state["dsp"].toObject();
        m_fft->getScaledSpectrum().setGain(dsp["gain"].toDouble(1.0));
        m_fft->getScaledSpectrum().setCompression(dsp["compression"].toDouble(1.0));
        m_fft->getScaledSpectrum().setDecibelConversion(dsp["decibel"].toBool(false));
        m_fft->getScaledSpectrum().setAgcEnabled(dsp["agc"].toBool(true));
    }
    
    // BPM Settings
    if (state.contains("bpm")) {
        QJsonObject bpm = state["bpm"].toObject();
        m_bpmDetector->setMinBPM(bpm["min"].toInt(75));
        m_bpmOsc->setBPMMute(bpm["mute"].toBool(false));
        if (bpm.contains("osc")) {
            m_bpmOsc->fromState(bpm["osc"].toObject());
        }
    }
    
    // Triggers
    if (state.contains("triggers")) {
        QJsonObject triggers = state["triggers"].toObject();
        if (triggers.contains("bass")) m_bass->fromState(triggers["bass"].toObject());
        if (triggers.contains("loMid")) m_loMid->fromState(triggers["loMid"].toObject());
        if (triggers.contains("hiMid")) m_hiMid->fromState(triggers["hiMid"].toObject());
        if (triggers.contains("high")) m_high->fromState(triggers["high"].toObject());
        if (triggers.contains("envelope")) m_envelope->fromState(triggers["envelope"].toObject());
        if (triggers.contains("silence")) m_silence->fromState(triggers["silence"].toObject());
    }
}

} // namespace sound2osc
