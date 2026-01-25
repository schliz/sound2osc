#include <QtTest>
#include "sound2osc/trigger/TriggerGenerator.h"
#include "sound2osc/dsp/ScaledSpectrum.h"
#include "sound2osc/osc/OSCNetworkManager.h"

class TestTrigger : public QObject
{
    Q_OBJECT

private slots:
    void testBandpassTrigger()
    {
        // 1. Setup TriggerGenerator
        // midFreq = 1000 Hz, isBandpass = true
        TriggerGenerator trigger("TestTrigger", nullptr, true, false, 1000);
        
        // Threshold 0.5
        trigger.setThreshold(0.5);
        trigger.setWidth(0.1); // Narrow band
        
        // 2. Setup Spectrum
        // Base freq 20Hz, 200 bins
        ScaledSpectrum spectrum(20, 200);
        
        // Linear spectrum size 2048 (typical from FFTAnalyzer)
        QVector<float> linearSpectrum(2048, 0.0f);
        
        // --- Case 1: Silence ---
        spectrum.updateWithLinearSpectrum(linearSpectrum);
        QVERIFY2(!trigger.checkForTrigger(spectrum, false), "Trigger fired on silence");
        
        // --- Case 2: Signal Out of Band (e.g., 100 Hz) ---
        // 100 Hz / 22050 * 2048 = ~9
        linearSpectrum[9] = 1.0f; // Max energy
        spectrum.updateWithLinearSpectrum(linearSpectrum);
        
        // Note: ScaledSpectrum scaling might be wide, so we assume 100Hz is far enough from 1000Hz.
        // With width 0.1, it should be.
        QVERIFY2(!trigger.checkForTrigger(spectrum, false), "Trigger fired on out-of-band signal");
        
        // --- Case 3: Signal In Band (1000 Hz) ---
        // 1000 Hz / 22050 * 2048 = ~93
        linearSpectrum.fill(0.0f); // Reset
        linearSpectrum[93] = 100.0f; // High energy (raw FFT value)
        spectrum.updateWithLinearSpectrum(linearSpectrum);
        
        // ScaledSpectrum will normalize/scale this.
        // Assuming default Gain/Compression allows it to pass 0.5 threshold.
        // Let's ensure gain is sufficient.
        spectrum.setGain(1.0f);
        spectrum.setCompression(1.0f);
        
        // Debug: check level directly
        // float level = spectrum.getMaxLevel(1000, 0.1);
        // qDebug() << "Level at 1000Hz:" << level;
        
        bool triggered = trigger.checkForTrigger(spectrum, false);
        // qDebug() << "Current Level:" << trigger.getCurrentLevel() << "Threshold:" << trigger.getThreshold();
        
        QVERIFY2(triggered, "Trigger failed to fire on in-band signal");
    }
};

QTEST_GUILESS_MAIN(TestTrigger)
#include "TestTrigger.moc"
