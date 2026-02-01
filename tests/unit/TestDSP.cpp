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

    bool checkForTrigger(const ScaledSpectrum& spectrum, bool forceRelease) override {
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

    QJsonObject toState() const override {
        return QJsonObject();
    }

    void fromState(const QJsonObject& state) override {
        Q_UNUSED(state);
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

    void testSquareWaveHarmonics()
    {
        // 1. Setup Buffer
        MonoAudioBuffer buffer(NUM_SAMPLES);
        
        // Generate 440 Hz square wave
        const int sampleRate = 44100;
        const double frequency = 430.66; // Align with bin
        
        QVector<qreal> samples(NUM_SAMPLES);
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double sineVal = qSin(2.0 * M_PI * frequency * t);
            samples[i] = (sineVal >= 0) ? 1.0 : -1.0;
        }
        
        buffer.putSamples(samples, 1);
        
        // 2. Analyze
        QVector<TriggerGeneratorInterface*> triggers;
        FFTAnalyzer fft(buffer, triggers);
        fft.calculateFFT(false);
        
        const QVector<float>& bins = fft.getScaledSpectrum().getNormalizedSpectrum();
        
        // 3. Verify Harmonics (Fundamental, 3rd, 5th)
        // Note: Bin index depends on mapping. 
        // We scan for peaks.
        
        int f1_bin = -1;
        float f1_val = 0.0f;
        
        // Find fundamental peak
        for(int i=0; i<bins.size(); ++i) {
            if(bins[i] > f1_val) {
                f1_val = bins[i];
                f1_bin = i;
            }
        }
        
        QVERIFY2(f1_val > 0.5f, "Fundamental frequency not detected strong enough");
        
        // We expect harmonics at odd multiples. 
        // Since mapping might be logarithmic or linear depending on ScaledSpectrum implementation,
        // we'll search locally for other peaks. 
        // Actually, ScaledSpectrum usually maps to a log/mel scale or specific bands.
        // Let's check if we find *some* other significant peaks which aren't the fundamental.
        
        int harmonic_count = 0;
        for(int i=0; i<bins.size(); ++i) {
            // Check for local maxima distinct from fundamental
            if (i == f1_bin || i == f1_bin-1 || i == f1_bin+1) continue;
            
            if (bins[i] > 0.1f) { // Arbitrary threshold for "significant harmonic"
                 harmonic_count++;
            }
        }
        
        QVERIFY2(harmonic_count > 0, "No harmonics detected for square wave");
        qDebug() << "Square Wave Harmonics detected count:" << harmonic_count;
    }

    void testWhiteNoiseFlatness()
    {
        // 1. Setup Buffer
        MonoAudioBuffer buffer(NUM_SAMPLES);
        QVector<qreal> samples(NUM_SAMPLES);
        
        // Generate White Noise
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            samples[i] = (QRandomGenerator::global()->generateDouble() * 2.0) - 1.0;
        }
        
        buffer.putSamples(samples, 1);
        
        // 2. Analyze
        QVector<TriggerGeneratorInterface*> triggers;
        FFTAnalyzer fft(buffer, triggers);
        fft.calculateFFT(false);
        
        const QVector<float>& bins = fft.getScaledSpectrum().getNormalizedSpectrum();
        
        // 3. Verify Flatness
        // In a perfectly flat spectrum (linear), noise is flat. 
        // In ScaledSpectrum (likely log/mel), it might slope, but shouldn't have single sharp peaks.
        // We check that the standard deviation isn't extremely high relative to mean,
        // or that max peak isn't "too much" higher than average.
        
        float sum = 0.0f;
        float maxVal = 0.0f;
        
        for (float v : bins) {
            sum += v;
            if (v > maxVal) maxVal = v;
        }
        
        float mean = sum / bins.size();
        
        // For white noise, energy is distributed. No single bin should hoard all energy.
        // For sine wave, maxVal would be ~1.0 and mean very low.
        // For noise, mean should be substantial relative to maxVal.
        
        qDebug() << "White Noise - Mean:" << mean << "Max:" << maxVal;
        
        QVERIFY2(mean > 0.0f, "White noise produced zero spectrum");
        
        // Ratio check: If Max is > 10x Mean, it's likely peaking (spiky). 
        // Noise is "rough" but distributed.
        QVERIFY2(maxVal < (mean * 15.0f), "Spectrum too spiky for white noise");
    }

    void testSilence()
    {
        MonoAudioBuffer buffer(NUM_SAMPLES);
        QVector<qreal> samples(NUM_SAMPLES);
        samples.fill(0.0);
        
        buffer.putSamples(samples, 1);
        
        QVector<TriggerGeneratorInterface*> triggers;
        FFTAnalyzer fft(buffer, triggers);
        fft.calculateFFT(false);
        
        const QVector<float>& bins = fft.getScaledSpectrum().getNormalizedSpectrum();
        
        float maxVal = 0.0f;
        for (float v : bins) if (v > maxVal) maxVal = v;
        
        QVERIFY2(maxVal < 0.001f, "Silence produced non-zero spectrum");
    }
};

QTEST_GUILESS_MAIN(TestDSP)
#include "TestDSP.moc"
