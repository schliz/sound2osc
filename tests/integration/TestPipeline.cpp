#include <QtTest>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "sound2osc/core/Sound2OscEngine.h"
#include "sound2osc/logging/Logger.h"
#include "sound2osc/audio/AudioInputInterface.h"

// Mock Audio Input to deterministically drive the engine
class MockAudioInput : public AudioInputInterface
{
public:
    explicit MockAudioInput(MonoAudioBuffer* buffer) : AudioInputInterface(buffer) {}
    
    void start() override { m_running = true; }
    void stop() override { m_running = false; }
    
    void setCallback(Callback callback) override { m_callback = callback; }
    QStringList getAvailableInputs() const override { return QStringList("MockInput"); }
    QString getActiveInputName() const override { return "MockInput"; }
    void setInputByName(const QString&) override {} // No-op
    qreal getVolume() const override { return 1.0; }
    void setVolume(const qreal&) override {}
    QString getDefaultInputName() const override { return "MockInput"; }

    // Helper to push data and trigger processing
    void pushData(QVector<qreal>& data) {
        if (!m_running) return;
        m_buffer->putSamples(data, 1);
        if (m_callback) {
            m_callback(static_cast<int>(data.size()));
        }
    }

private:
    Callback m_callback;
    bool m_running{false};
};

class TestPipeline : public QObject
{
    Q_OBJECT

private slots:
    void testFullPipeline()
    {
        // 1. Setup UDP Receiver (Mock OSC Target)
        QUdpSocket receiver;
        bool bound = receiver.bind(QHostAddress::LocalHost, 9000); // Use 9000 for test
        QVERIFY2(bound, "Could not bind UDP receiver port 9000");
        
        // 2. Setup Engine
        auto settings = std::make_shared<sound2osc::SettingsManager>();
        
        // Configure Engine to send to our receiver
        settings->setOscIpAddress("127.0.0.1");
        settings->setOscUdpTxPort(9000);
        settings->setOscEnabled(true);
        settings->setUseTcp(false);
        
        sound2osc::Sound2OscEngine engine(settings);
        
        // Inject Mock Audio Input
        auto mockInputPtr = std::make_unique<MockAudioInput>(engine.getAudioBuffer());
        MockAudioInput* mockInput = mockInputPtr.get();
        engine.setAudioInput(std::move(mockInputPtr));
        
        engine.start();
        
        // Ensure Bass Trigger is sensitive
        TriggerGenerator* bass = engine.getBass();
        bass->setThreshold(0.001); // Very sensitive
        bass->getOscParameters().setLevelMessage("/bass/level "); // Note space
        
        // 3. Inject Audio (Bass Kick)
        const int chunkSize = 1024;
        const int totalSamples = 44100 * 2; // 2 seconds
        int currentSample = 0;
        
        bool receivedMessage = false;
        
        // Loop to process audio
        while (currentSample < totalSamples && !receivedMessage) {
            QVector<qreal> chunk(chunkSize);
            for(int i=0; i<chunkSize; ++i) {
                // 50 Hz Sine Wave
                float t = static_cast<float>(currentSample + i) / 44100.0f;
                chunk[i] = qSin(2.0f * M_PI * 50.0f * t);
            }
            
            // Push data via Mock Input - this triggers the engine loop directly!
            mockInput->pushData(chunk);
            
            // Process events to let Engine timers/slots fire
            QCoreApplication::processEvents();
            
            // Check Receiver
            while (receiver.hasPendingDatagrams()) {
                QNetworkDatagram datagram = receiver.receiveDatagram();
                QString msg = QString::fromUtf8(datagram.data());
                
                // Check if it's our bass trigger
                if (msg.startsWith("/bass/level")) {
                    receivedMessage = true;
                    qDebug() << "Received OSC:" << msg;
                }
            }
            
            currentSample += chunkSize;
        }
        
        QVERIFY2(receivedMessage, "Did not receive OSC message for bass signal");
        
        engine.stop();
    }
};

QTEST_GUILESS_MAIN(TestPipeline)
#include "TestPipeline.moc"
