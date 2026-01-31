#include <QtTest>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "sound2osc/core/Sound2OscEngine.h"
#include "sound2osc/logging/Logger.h"

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
        engine.start();
        
        // Ensure Bass Trigger is sensitive
        TriggerGenerator* bass = engine.getBass();
        bass->setThreshold(0.1);
        bass->getOscParameters().setLevelMessage("/bass/level "); // Note space
        
        // 3. Inject Audio (Bass Kick)
        MonoAudioBuffer* buffer = engine.getAudioBuffer();
        
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
            
            buffer->putSamples(chunk, 1);
            
            // Process events to let Engine timers fire
            QTest::qWait(23); // ~44Hz frame is 23ms
            
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

QTEST_MAIN(TestPipeline)
#include "TestPipeline.moc"
