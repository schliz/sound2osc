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

    void testTimeDomainBehavior()
    {
        // 1. Setup
        TriggerGenerator trigger("TestTrigger", nullptr, false, false); // Envelope trigger
        trigger.setThreshold(0.5);
        
        // Configure Delays
        // Use substantial delays to be robust against test timing jitters
        double delayTime = 0.2; // 200ms
        trigger.getTriggerFilter().setOnDelay(delayTime);
        trigger.getTriggerFilter().setOffDelay(delayTime);
        
        // Spy on the filter signals
        QSignalSpy spyOn(&trigger.getTriggerFilter(), &TriggerFilter::onSignalSent);
        QSignalSpy spyOff(&trigger.getTriggerFilter(), &TriggerFilter::offSignalSent);
        
        QVERIFY(spyOn.isValid());
        QVERIFY(spyOff.isValid());
        
        // 2. Setup Spectrum (Input)
        ScaledSpectrum spectrum(20, 200);
        QVector<float> strongSignal(2048, 100.0f); // High energy
        QVector<float> silence(2048, 0.0f);
        
        spectrum.setGain(1.0f);
        spectrum.setCompression(1.0f);
        
        // 3. Test On Delay
        // Provide signal
        spectrum.updateWithLinearSpectrum(strongSignal);
        trigger.checkForTrigger(spectrum, false); // First detection
        
        // Should NOT be on yet due to delay
        QCOMPARE(spyOn.count(), 0);
        
        // Wait half the delay
        QTest::qWait(100); 
        // Need to keep "poking" the generator? 
        // No, TriggerFilter uses QTimer, independent of checkForTrigger loop once activated.
        // However, TriggerGenerator::checkForTrigger calls filter.triggerOn() repeatedly.
        // Let's verify if TriggerFilter handles re-triggering correctly (it should, usually restarting or ignoring timer).
        // Actually, usually triggerOn() is idempotent or re-triggering logic depends on implementation.
        // If it uses QTimer::start(), it resets the timer! This would be a bug if called every frame.
        // Let's assume the implementation checks state.
        
        // Let's simulate the loop behavior (calling checkForTrigger continuously)
        trigger.checkForTrigger(spectrum, false);
        
        QCOMPARE(spyOn.count(), 0); // Still waiting
        
        // Wait rest of delay + buffer
        QTest::qWait(150);
        
        QCOMPARE(spyOn.count(), 1); // Should have fired
        
        // 4. Test Off Delay
        // Remove signal
        spectrum.updateWithLinearSpectrum(silence);
        trigger.checkForTrigger(spectrum, false);
        
        QCOMPARE(spyOff.count(), 0); // Should be waiting
        
        QTest::qWait(100);
        trigger.checkForTrigger(spectrum, false);
        QCOMPARE(spyOff.count(), 0);
        
        QTest::qWait(150);
        QCOMPARE(spyOff.count(), 1); // Should have released
    }

    void testExtremeThresholds()
    {
        TriggerGenerator trigger("TestTrigger", nullptr, false, false);
        ScaledSpectrum spectrum(20, 200);
        QVector<float> mediumSignal(2048, 5.0f); // Moderate signal
        spectrum.updateWithLinearSpectrum(mediumSignal);
        
        // Threshold 0.0 -> Always Trigger
        trigger.setThreshold(0.0);
        bool fired = trigger.checkForTrigger(spectrum, false);
        QVERIFY2(fired, "Threshold 0.0 should always fire");
        
        // Threshold 1.0 -> Never Trigger (unless signal is infinite/clipping max)
        trigger.setThreshold(1.0);
        QCOMPARE(trigger.getThreshold(), 1.0);
        
        // Ensure signal is small enough and AGC is OFF
        spectrum.setAgcEnabled(false);

        spectrum.setGain(0.1f); 
        
        // Use a very weak signal
        QVector<float> weakSignal(2048, 0.01f);
        spectrum.updateWithLinearSpectrum(weakSignal);
        
        qDebug() << "Level:" << trigger.getCurrentLevel() << "Threshold:" << trigger.getThreshold();

        fired = trigger.checkForTrigger(spectrum, false);
        qDebug() << "After Check - Level:" << trigger.getCurrentLevel() << "Fired:" << fired;

        QVERIFY2(!fired, "Threshold 1.0 should not fire with weak signal and AGC off");
    }
};

QTEST_GUILESS_MAIN(TestTrigger)
#include "TestTrigger.moc"
