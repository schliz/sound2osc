#include <QtTest>
#include <QtMath>
#include "sound2osc/dsp/FFTAnalyzer.h"
#include "sound2osc/audio/MonoAudioBuffer.h"
#include "sound2osc/trigger/TriggerGeneratorInterface.h"
#include "sound2osc/trigger/TriggerFilter.h"
#include "sound2osc/trigger/TriggerOscParameters.h"

// Dummy trigger generator for testing
class DummyTrigger : public TriggerGeneratorInterface {
public:
    DummyTrigger() 
        : TriggerGeneratorInterface(false)
        , m_params()
        , m_filter(nullptr, m_params, false) 
    {}

    bool checkForTrigger(ScaledSpectrum& spectrum, bool forceRelease) override {
        Q_UNUSED(spectrum);
        Q_UNUSED(forceRelease);
        return false;
    }
    
    TriggerFilter& getTriggerFilter() override {
        return m_filter;
    }
    
    void save(QSettings& settings) const override {
        Q_UNUSED(settings);
    }
    
    void restore(QSettings& settings) override {
        Q_UNUSED(settings);
    }

private:
    TriggerOscParameters m_params;
    TriggerFilter m_filter;
};

class TestDSP : public QObject
{
    Q_OBJECT

private slots:
    void testSineWaveFFT()
    {
        // 1. Setup Buffer
        // FFTAnalyzer uses NUM_SAMPLES (4096)
        MonoAudioBuffer buffer(NUM_SAMPLES);
        
        // Generate 440 Hz sine wave at 44100 Hz sample rate
        const int sampleRate = 44100;
        const double frequency = 430.66; // Chosen to align with bin center
        
        QVector<qreal> samples;
        samples.resize(NUM_SAMPLES);
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            samples[i] = qSin(2.0 * M_PI * frequency * t);
        }
        
        buffer.putSamples(samples, 1);
        
        // 2. Setup Dependencies
        QVector<TriggerGeneratorInterface*> triggers;
        DummyTrigger dummy;
        triggers.append(&dummy);
        
        FFTAnalyzer fft(buffer, triggers);
        
        // 3. Run Analysis
        fft.calculateFFT(false);
        
        // 4. Verify Spectrum
        const ScaledSpectrum& spectrum = fft.getScaledSpectrum();
        const QVector<float>& bins = spectrum.getNormalizedSpectrum();
        
        // 5. Verify Signal
        bool hasSignal = false;
        float maxVal = 0.0f;
        int maxBin = -1;
        
        for (int i = 0; i < bins.size(); ++i) {
            if (bins[i] > 0.01f) hasSignal = true;
            if (bins[i] > maxVal) {
                maxVal = bins[i];
                maxBin = i;
            }
        }
        
        QVERIFY2(hasSignal, "FFT produced no signal for sine wave input");
        
        // Output debug info
        qDebug() << "TestDSP: Peak at bin:" << maxBin << "Value:" << maxVal;
    }
};

QTEST_GUILESS_MAIN(TestDSP)
#include "TestDSP.moc"
