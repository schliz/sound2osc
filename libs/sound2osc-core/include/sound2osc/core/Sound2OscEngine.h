// SPDX-License-Identifier: MIT
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
//
// Sound2OscEngine - Unified Core Processing Engine

#ifndef SOUND2OSC_CORE_SOUND2OSCENGINE_H
#define SOUND2OSC_CORE_SOUND2OSCENGINE_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QJsonObject>
#include <memory>

#include <sound2osc/audio/MonoAudioBuffer.h>
#include <sound2osc/audio/AudioInputInterface.h>
#include <sound2osc/dsp/FFTAnalyzer.h>
#include <sound2osc/osc/OSCNetworkManager.h>
#include <sound2osc/bpm/BPMDetector.h>
#include <sound2osc/bpm/BPMOscControler.h>
#include <sound2osc/trigger/TriggerGenerator.h>
#include <sound2osc/config/ConfigStore.h>
#include <sound2osc/config/SettingsManager.h>

#include <atomic>

namespace sound2osc {
// class QAudioInputWrapper; // Removed as we use the interface base class

/**
 * @brief The central engine that orchestrates audio analysis and OSC generation.
 * 
 * This class encapsulates the entire processing pipeline:
 * Audio Input -> Buffer -> FFT -> Triggers/BPM -> OSC Output
 * 
 * It manages the lifecycle of all core components and the main processing loops.
 */
class Sound2OscEngine : public QObject
{
    Q_OBJECT

public:
    explicit Sound2OscEngine(std::shared_ptr<SettingsManager> settings, QObject *parent = nullptr);
    ~Sound2OscEngine() override;

    /**
     * @brief Start the processing engine
     * Initializes audio input and starts processing timers.
     */
    void start();

    /**
     * @brief Stop the processing engine
     * Stops audio input and processing timers.
     */
    void stop();

    // -- Component Accessors --
    
    OSCNetworkManager* osc() { return m_osc.get(); }
    AudioInputInterface* audioInput() { return m_audioInput.get(); }
    MonoAudioBuffer* getAudioBuffer() { return m_audioBuffer.get(); }
    FFTAnalyzer* fft() { return m_fft.get(); }
    BPMDetector* bpm() { return m_bpmDetector.get(); }
    BPMOscControler* bpmOsc() { return m_bpmOsc.get(); }
    
    // Trigger Generators access
    QVector<TriggerGeneratorInterface*>& getTriggers() { return m_triggerInterfaces; }
    
    // Individual access for UI binding (legacy support)
    TriggerGenerator* getBass() { return m_bass.get(); }
    TriggerGenerator* getLoMid() { return m_loMid.get(); }
    TriggerGenerator* getHiMid() { return m_hiMid.get(); }
    TriggerGenerator* getHigh() { return m_high.get(); }
    TriggerGenerator* getEnvelope() { return m_envelope.get(); }
    TriggerGenerator* getSilence() { return m_silence.get(); }

    // -- Configuration --
    
    void setLowSoloMode(bool enabled);
    bool getLowSoloMode() const { return m_lowSoloMode; }

    // -- Preset State Management --
    
    /**
     * @brief Serialize the complete engine state (triggers, BPM settings, etc.) to JSON
     */
    QJsonObject toState() const;

    /**
     * @brief Restore engine state from JSON
     */
    void fromState(const QJsonObject& state);

    /**
     * @brief Inject a custom Audio Input backend (e.g. for testing)
     * Must be called before start().
     * Takes ownership of the pointer.
     */
    void setAudioInput(std::unique_ptr<AudioInputInterface> input);

public slots:
    // Apply settings from SettingsManager to components
    void applySettings();

private slots:
    void onFftTimer();
    void onBpmTimer();
    void onStatusTimer();

private:
    void initializeComponents();
    void connectComponents();
    void onAudioProcessed(int count);

    bool m_running;
    bool m_lowSoloMode;
    std::atomic<int> m_accumulatedSamples{0};

    std::shared_ptr<SettingsManager> m_settings;

    // Components
    std::unique_ptr<MonoAudioBuffer> m_audioBuffer;
    std::unique_ptr<AudioInputInterface> m_audioInput;
    std::unique_ptr<OSCNetworkManager> m_osc;
    std::unique_ptr<BPMOscControler> m_bpmOsc;
    std::unique_ptr<BPMDetector> m_bpmDetector;
    
    // Trigger Generators
    std::unique_ptr<TriggerGenerator> m_bass;
    std::unique_ptr<TriggerGenerator> m_loMid;
    std::unique_ptr<TriggerGenerator> m_hiMid;
    std::unique_ptr<TriggerGenerator> m_high;
    std::unique_ptr<TriggerGenerator> m_envelope;
    std::unique_ptr<TriggerGenerator> m_silence;
    
    // Container for FFTAnalyzer (which expects raw pointers)
    QVector<TriggerGeneratorInterface*> m_triggerInterfaces;
    
    std::unique_ptr<FFTAnalyzer> m_fft;

    // Timers
    // QTimer m_fftTimer; // Removed in favor of event-driven loop
    QTimer m_bpmTimer;
    QTimer m_statusTimer;
};

} // namespace sound2osc

#endif // SOUND2OSC_CORE_SOUND2OSCENGINE_H
