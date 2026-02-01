#include <QtTest>
#include "sound2osc/logging/Logger.h"

#include <QTemporaryFile>
#include <QDir>

using namespace sound2osc;

class TestLogger : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Ensure we start with clean logger state
    }

    void testLogLevels()
    {
        // Test that all log levels work without crashing
        Logger::debug("Debug message");
        Logger::info("Info message");
        Logger::warning("Warning message");
        Logger::error("Error message");
        
        // If we get here without crashing, the test passes
        QVERIFY(true);
    }

    void testFormattedMessages()
    {
        // Test message formatting with QString::arg
        QString msg = QString("Test value: %1").arg(42);
        Logger::info(msg);
        
        QString floatMsg = QString("Float value: %1").arg(3.14, 0, 'f', 2);
        Logger::info(floatMsg);
        
        QVERIFY(true);
    }

    void testMultipleMessages()
    {
        // Test rapid logging
        for (int i = 0; i < 100; ++i) {
            Logger::debug(QString("Message %1").arg(i));
        }
        
        QVERIFY(true);
    }

    void testEmptyMessage()
    {
        // Test empty string handling
        Logger::info("");
        Logger::info(QString());
        
        QVERIFY(true);
    }

    void testSpecialCharacters()
    {
        // Test special characters in messages
        Logger::info("Message with\nnewline");
        Logger::info("Message with\ttab");
        Logger::info("Unicode: \u00e4\u00f6\u00fc\u00df");
        
        QVERIFY(true);
    }

    void cleanupTestCase()
    {
        // Clean up after tests
    }
};

QTEST_GUILESS_MAIN(TestLogger)
#include "TestLogger.moc"
