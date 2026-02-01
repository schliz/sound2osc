#include <QtTest>
#include "sound2osc/core/AppInfo.h"

using namespace sound2osc;

class TestAppInfo : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Reset to defaults before each test suite
        AppInfo::resetToDefaults();
    }

    void testDefaultOrganizationName()
    {
        QCOMPARE(AppInfo::organizationName(), QString("sound2osc"));
    }

    void testSetOrganizationName()
    {
        AppInfo::setOrganizationName("TestOrg");
        QCOMPARE(AppInfo::organizationName(), QString("TestOrg"));
        AppInfo::resetToDefaults();
    }

    void testDefaultApplicationName()
    {
        QCOMPARE(AppInfo::applicationName(), QString("sound2osc"));
    }

    void testSetApplicationName()
    {
        AppInfo::setApplicationName("TestApp");
        QCOMPARE(AppInfo::applicationName(), QString("TestApp"));
        AppInfo::resetToDefaults();
    }

    void testApplicationDisplayName()
    {
        QCOMPARE(AppInfo::applicationDisplayName(), QString("sound2osc"));
    }

    void testApplicationVersion()
    {
        // Version should not be empty
        QVERIFY(!AppInfo::applicationVersion().isEmpty());
    }

    void testPresetFileExtension()
    {
        QCOMPARE(AppInfo::presetFileExtension(), QString("s2o"));
    }

    void testSetPresetFileExtension()
    {
        AppInfo::setPresetFileExtension("custom");
        QCOMPARE(AppInfo::presetFileExtension(), QString("custom"));
        AppInfo::resetToDefaults();
    }

    void testAutosaveFileExtension()
    {
        QCOMPARE(AppInfo::autosaveFileExtension(), QString("ats"));
    }

    void testConfigFileName()
    {
        QCOMPARE(AppInfo::configFileName(), QString("config.json"));
    }

    void testSupportedConsoleTypes()
    {
        QStringList types = AppInfo::supportedConsoleTypes();
        QVERIFY(types.contains("Eos"));
        QVERIFY(types.contains("Cobalt 7.2"));
    }

    void testDefaultConsoleType()
    {
        QCOMPARE(AppInfo::defaultConsoleType(), QString("Eos"));
    }

    void testLegacyOrganizationName()
    {
        QCOMPARE(AppInfo::legacyOrganizationName(), QString(""));
    }

    void testLegacyApplicationName()
    {
        QCOMPARE(AppInfo::legacyApplicationName(), QString("sound2osc"));
    }

    void testResetToDefaults()
    {
        // Modify some values
        AppInfo::setOrganizationName("Modified");
        AppInfo::setApplicationName("Modified");
        AppInfo::setPresetFileExtension("mod");
        
        // Reset
        AppInfo::resetToDefaults();
        
        // Verify defaults are restored
        QCOMPARE(AppInfo::organizationName(), QString("sound2osc"));
        QCOMPARE(AppInfo::applicationName(), QString("sound2osc"));
        QCOMPARE(AppInfo::presetFileExtension(), QString("s2o"));
    }

    void cleanupTestCase()
    {
        AppInfo::resetToDefaults();
    }
};

QTEST_GUILESS_MAIN(TestAppInfo)
#include "TestAppInfo.moc"
