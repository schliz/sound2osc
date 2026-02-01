#include <QtTest>
#include <QtMath>
#include <QRandomGenerator>
#include "sound2osc/bpm/BPMDetector.h"
#include "sound2osc/bpm/BPMOscControler.h"
#include "sound2osc/audio/MonoAudioBuffer.h"
#include "sound2osc/osc/OSCNetworkManager.h"

class TestBPM : public QObject
{
    Q_OBJECT

private slots:
    void testBPMDetection()
    {
        // 1. Setup Dependencies
        OSCNetworkManager osc;
        BPMOscControler bpmOsc(osc);
        MonoAudioBuffer buffer(4096);
        
        BPMDetector bpmDetector(buffer, &bpmOsc);
        
        // 2. Simulate Audio (120 BPM)
        const int sampleRate = 44100;
        const int simulationDuration = 10; // seconds
        const int chunkSize = 1024; 
        
        int totalSamples = sampleRate * simulationDuration;
        int currentSample = 0;
        
        QVector<qreal> chunk;
        chunk.resize(chunkSize);
        
        // Initial detection
        bpmDetector.resetCache();
        
        while (currentSample < totalSamples) {
            // Fill chunk
            for (int i = 0; i < chunkSize; ++i) {
                int globalIndex = currentSample + i;
                
                // Simulate Kick Drum: Sine wave sweep (50Hz) with envelope
                // 120 BPM = 2 beats/sec = 22050 samples interval
                int positionInBeat = globalIndex % 22050;
                float sample = 0.0f;
                
                if (positionInBeat < 2000) {
                    float t = static_cast<float>(positionInBeat) / sampleRate;
                    sample = qSin(2.0f * M_PI * 100.0f * t); // 100Hz kick
                    sample *= qMax(0.0f, 1.0f - (float)positionInBeat/2000.0f);
                }
                
                // Add some noise
                sample += (static_cast<float>(QRandomGenerator::global()->generateDouble()) - 0.5f) * 0.1f;
                
                chunk[i] = sample;
            }
            
            // Push to buffer
            buffer.putSamples(chunk, 1);
            
            // Run detection
            bpmDetector.detectBPM();
            
            currentSample += chunkSize;
        }
        
        // 3. Verify
        float detected = bpmDetector.getBPM();
        qDebug() << "Detected BPM:" << detected;
        
        if (detected == 0.0f) {
             const auto& flux = bpmDetector.getWaveDisplay();
             float maxFlux = 0.0f;
             for(float f : flux) maxFlux = qMax(maxFlux, f);
             qDebug() << "Max Flux:" << maxFlux << "Buffer Size:" << flux.count();
        }

        QVERIFY(detected > 110.0f && detected < 130.0f);
    }

    void testBPMStepChange()
    {
        // 1. Setup
        OSCNetworkManager osc;
        BPMOscControler bpmOsc(osc);
        MonoAudioBuffer buffer(4096);
        BPMDetector bpmDetector(buffer, &bpmOsc);
        
        const int sampleRate = 44100;
        const int chunkSize = 1024;
        
        // Helper to run simulation
        struct SimulationState {
            int64_t globalSampleIndex = 0;
            QVector<qreal> chunk;
            SimulationState(int size) : chunk(size) {}
        } state(chunkSize);

        auto processAudio = [&](float targetBPM, int durationSec) {
             int samplesToProcess = sampleRate * durationSec;
             int processed = 0;
             int beatInterval = static_cast<int>(sampleRate * 60.0f / targetBPM);
             
             while(processed < samplesToProcess) {
                 for(int i=0; i<chunkSize; ++i) {
                     int pos = (state.globalSampleIndex) % beatInterval;
                     float sample = 0.0f;
                     // Kick drum: Sharp attack
                     if (pos < 2000) {
                         float t = static_cast<float>(pos) / sampleRate;
                         sample = qSin(2.0f * M_PI * 100.0f * t); // 100Hz kick
                         // Envelope
                         sample *= qMax(0.0f, 1.0f - (float)pos/2000.0f);
                     }
                     // Add noise to ensure variance
                     sample += (static_cast<float>(QRandomGenerator::global()->generateDouble()) - 0.5f) * 0.1f;
                     
                     state.chunk[i] = sample;
                     state.globalSampleIndex++;
                 }
                 
                 buffer.putSamples(state.chunk, 1);
                 bpmDetector.detectBPM();
                 processed += chunkSize;
             }
        };
        
        // Phase 1: 100 BPM
        processAudio(100.0f, 10);
        float bpm1 = bpmDetector.getBPM();
        
        if (bpm1 == 0.0f) {
             const auto& flux = bpmDetector.getWaveDisplay();
             float maxFlux = 0.0f;
             for(float f : flux) maxFlux = qMax(maxFlux, f);
             qDebug() << "Max Flux:" << maxFlux << "Buffer Size:" << flux.count();
        }

        qDebug() << "BPM after 100:" << bpm1;
        QVERIFY(bpm1 > 90.0f && bpm1 < 110.0f);
        
        // Phase 2: 120 BPM
        processAudio(120.0f, 10);
        float bpm2 = bpmDetector.getBPM();
        qDebug() << "BPM after 120:" << bpm2;
        QVERIFY(bpm2 > 110.0f && bpm2 < 130.0f);
    }
};

QTEST_GUILESS_MAIN(TestBPM)
#include "TestBPM.moc"
