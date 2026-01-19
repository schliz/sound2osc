#include <QtTest>
#include "sound2osc/osc/OSCMessage.h"
#include "sound2osc/osc/OSCParser.h"

class TestOSCMessage : public QObject
{
    Q_OBJECT

private slots:
    void testParsing()
    {
        // 1. Create a raw OSC packet using OSCPacketWriter
        OSCPacketWriter writer("/test/address");
        writer.AddInt32(42);
        writer.AddFloat32(3.14f);
        writer.AddString("hello");

        size_t size;
        char* rawData = writer.Create(size);
        QByteArray data(rawData, static_cast<int>(size));
        delete[] rawData; // OSCPacketWriter::Create allocates memory

        // 2. Parse it with OSCMessage
        OSCMessage msg(data);

        // 3. Verify
        QVERIFY(msg.isValid());
        QCOMPARE(msg.pathString(), QString("/test/address"));
        
        const QVector<QVariant>& args = msg.arguments();
        QCOMPARE(args.size(), 3);
        QCOMPARE(args[0].toInt(), 42);
        // Compare float with tolerance
        QVERIFY(qAbs(args[1].toFloat() - 3.14f) < 0.0001f); 
        QCOMPARE(args[2].toString(), QString("hello"));
    }
};

QTEST_GUILESS_MAIN(TestOSCMessage)
#include "TestOSCMessage.moc"
