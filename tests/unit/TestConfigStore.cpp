#include <QtTest>
#include "sound2osc/config/JsonConfigStore.h"

#include <QTemporaryFile>
#include <QTemporaryDir>

using namespace sound2osc;

class TestConfigStore : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    QString m_testConfigPath;

private slots:
    void initTestCase()
    {
        QVERIFY(m_tempDir.isValid());
        m_testConfigPath = m_tempDir.path() + "/test_config.json";
    }

    void testCreateAndSave()
    {
        JsonConfigStore store(m_testConfigPath);
        
        // Set some values
        store.setValue("test/string", QString("hello"));
        store.setValue("test/int", 42);
        store.setValue("test/bool", true);
        store.setValue("test/double", 3.14);
        
        // Save
        QVERIFY(store.save());
        
        // Check file was created
        QVERIFY(QFile::exists(m_testConfigPath));
    }

    void testLoadAndRetrieve()
    {
        // First create a config
        {
            JsonConfigStore store(m_testConfigPath);
            store.setValue("load/test", QString("test_value"));
            store.setValue("load/number", 123);
            store.save();
        }
        
        // Now load it in a new instance
        JsonConfigStore store(m_testConfigPath);
        QVERIFY(store.load());
        
        // Retrieve values
        QCOMPARE(store.getValue("load/test").toString(), QString("test_value"));
        QCOMPARE(store.getValue("load/number").toInt(), 123);
    }

    void testDefaultValues()
    {
        JsonConfigStore store(m_tempDir.path() + "/nonexistent.json");
        
        // Test default value for non-existent key
        QCOMPARE(store.getValue("nonexistent", 42).toInt(), 42);
        QCOMPARE(store.getValue("nonexistent", QString("default")).toString(), QString("default"));
    }

    void testContains()
    {
        JsonConfigStore store(m_testConfigPath);
        store.setValue("exists/key", true);
        
        QVERIFY(store.contains("exists/key"));
        QVERIFY(!store.contains("doesnotexist"));
    }

    void testRemove()
    {
        JsonConfigStore store(m_testConfigPath);
        store.setValue("remove/test", QString("to_remove"));
        QVERIFY(store.contains("remove/test"));
        
        store.remove("remove/test");
        QVERIFY(!store.contains("remove/test"));
    }

    void testGroupValues()
    {
        JsonConfigStore store(m_testConfigPath);
        
        // Set group values
        store.setGroupValue("osc", "ipAddress", QString("192.168.1.1"));
        store.setGroupValue("osc", "port", 9000);
        
        // Get group values
        QCOMPARE(store.getGroupValue("osc", "ipAddress").toString(), QString("192.168.1.1"));
        QCOMPARE(store.getGroupValue("osc", "port").toInt(), 9000);
    }

    void testIsDirty()
    {
        JsonConfigStore store(m_testConfigPath);
        store.load();
        QVERIFY(!store.isDirty());
        
        store.setValue("dirty/test", true);
        QVERIFY(store.isDirty());
        
        store.save();
        QVERIFY(!store.isDirty());
    }

    void testBackendType()
    {
        JsonConfigStore store(m_testConfigPath);
        QCOMPARE(store.getBackendType(), QString("json"));
    }

    void testStoragePath()
    {
        JsonConfigStore store(m_testConfigPath);
        QCOMPARE(store.getStoragePath(), m_testConfigPath);
    }

    void cleanupTestCase()
    {
        // Temp dir auto-cleans
    }
};

QTEST_GUILESS_MAIN(TestConfigStore)
#include "TestConfigStore.moc"
