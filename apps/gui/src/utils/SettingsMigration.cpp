// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT

#include "SettingsMigration.h"

#include <sound2osc/config/SettingsManager.h>
#include <sound2osc/config/PresetManager.h>
#include <sound2osc/core/AppInfo.h>
#include <sound2osc/logging/Logger.h>

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>

namespace sound2osc {

// Key used to mark migration as complete
static const char* MIGRATION_COMPLETE_KEY = "migration/v2_complete";

bool SettingsMigration::hasLegacySettings()
{
    // Check if legacy QSettings file exists
    QString legacyPath = legacySettingsPath();
    if (legacyPath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(legacyPath);
    if (!fileInfo.exists()) {
        return false;
    }
    
    // Check if already migrated
    if (migrationCompleted()) {
        return false;
    }
    
    // Verify it has valid content
    QSettings legacySettings(AppInfo::legacyOrganizationName(),
                             AppInfo::legacyApplicationName());
    int formatVersion = legacySettings.value("formatVersion", 0).toInt();
    return formatVersion > 0;
}

bool SettingsMigration::migrationCompleted()
{
    QSettings marker(AppInfo::organizationName(), AppInfo::applicationName());
    return marker.value(MIGRATION_COMPLETE_KEY, false).toBool();
}

void SettingsMigration::markMigrationComplete()
{
    QSettings marker(AppInfo::organizationName(), AppInfo::applicationName());
    marker.setValue(MIGRATION_COMPLETE_KEY, true);
    marker.setValue("migration/v2_date", QDateTime::currentDateTime().toString(Qt::ISODate));
    marker.sync();
}

bool SettingsMigration::migrate(SettingsManager* settingsManager,
                                PresetManager* presetManager)
{
    Logger::info("Starting settings migration from QSettings to JSON...");
    
    // Backup first
    QString backupPath = backupLegacySettings();
    if (backupPath.isEmpty()) {
        Logger::warning("Could not backup legacy settings, continuing anyway...");
    } else {
        Logger::info(QString("Legacy settings backed up to: %1").arg(backupPath));
    }
    
    // Migrate application settings
    bool settingsOk = migrateSettings(settingsManager);
    if (!settingsOk) {
        Logger::warning("Some settings migration issues occurred");
    }
    
    // Migrate presets (INI to JSON)
    bool presetsOk = migratePresets(presetManager);
    if (!presetsOk) {
        Logger::warning("Some preset migration issues occurred");
    }
    
    // Mark as complete even if partial success
    markMigrationComplete();
    Logger::info("Settings migration completed");
    
    return settingsOk && presetsOk;
}

bool SettingsMigration::migrateSettings(SettingsManager* settingsManager)
{
    if (!settingsManager) {
        Logger::error("SettingsManager is null, cannot migrate settings");
        return false;
    }
    
    QSettings legacy(AppInfo::legacyOrganizationName(),
                     AppInfo::legacyApplicationName());
    
    int formatVersion = legacy.value("formatVersion", 0).toInt();
    if (formatVersion == 0) {
        Logger::info("No legacy settings to migrate");
        return true;
    }
    
    Logger::info(QString("Migrating settings from format version %1").arg(formatVersion));
    
    // Migrate OSC settings
    if (legacy.contains("oscIpAddress")) {
        settingsManager->setOscIpAddress(legacy.value("oscIpAddress").toString());
    }
    if (legacy.contains("oscTxPort")) {
        settingsManager->setOscUdpTxPort(static_cast<quint16>(legacy.value("oscTxPort").toInt()));
    }
    if (legacy.contains("oscRxPort")) {
        settingsManager->setOscUdpRxPort(static_cast<quint16>(legacy.value("oscRxPort", 8000).toInt()));
    }
    if (legacy.contains("oscTcpPort")) {
        settingsManager->setOscTcpPort(static_cast<quint16>(legacy.value("oscTcpPort", 3032).toInt()));
    }
    if (legacy.contains("oscIsEnabled")) {
        settingsManager->setOscEnabled(legacy.value("oscIsEnabled").toBool());
    }
    if (legacy.contains("oscUseTcp")) {
        settingsManager->setUseTcp(legacy.value("oscUseTcp").toBool());
    }
    if (legacy.contains("oscUse_1_1")) {
        settingsManager->setUseOsc_1_1(legacy.value("oscUse_1_1").toBool());
    }
    
    // Migrate OSC logging settings
    if (legacy.value("oscLogSettingsValid").toBool()) {
        settingsManager->setOscLogIncomingEnabled(legacy.value("oscLogIncomingIsEnabled", true).toBool());
        settingsManager->setOscLogOutgoingEnabled(legacy.value("oscLogOutgoingIsEnabled", true).toBool());
    }
    
    // Migrate OSC input settings
    if (legacy.value("oscInputEnabledValid").toBool()) {
        settingsManager->setOscInputEnabled(legacy.value("oscInputEnabled", true).toBool());
    }
    
    // Migrate window geometry
    if (legacy.contains("windowGeometry")) {
        settingsManager->setWindowGeometry(legacy.value("windowGeometry").toRect());
    }
    if (legacy.contains("maximized")) {
        settingsManager->setWindowMaximized(legacy.value("maximized").toBool());
    }
    
    // Migrate input device
    if (legacy.contains("inputDeviceName")) {
        settingsManager->setInputDeviceName(legacy.value("inputDeviceName").toString());
    }
    
    // Save migrated settings
    settingsManager->save();
    
    Logger::info("Application settings migrated successfully");
    return true;
}

bool SettingsMigration::migratePresets(PresetManager* presetManager)
{
    if (!presetManager) {
        Logger::error("PresetManager is null, cannot migrate presets");
        return false;
    }
    
    QString presetDir = legacyPresetDirectory();
    QDir dir(presetDir);
    
    if (!dir.exists()) {
        Logger::info("No legacy preset directory found");
        return true;
    }
    
    // Find all .s2l files
    QStringList filters;
    filters << "*.s2l";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    if (files.isEmpty()) {
        Logger::info("No legacy preset files found");
        return true;
    }
    
    Logger::info(QString("Found %1 preset files to migrate").arg(files.size()));
    
    int successCount = 0;
    for (const QFileInfo& fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        
        // Load legacy preset
        QSettings legacyPreset(filePath, QSettings::IniFormat);
        int formatVersion = legacyPreset.value("formatVersion", 0).toInt();
        
        if (formatVersion == 0) {
            Logger::warning(QString("Skipping invalid preset: %1").arg(fileInfo.fileName()));
            continue;
        }
        
        // Create PresetData from legacy values
        PresetData data;
        data.decibelConversion = legacyPreset.value("dbConversion", false).toBool();
        data.fftGain = legacyPreset.value("fftGain", 1.0).toDouble();
        data.fftCompression = legacyPreset.value("fftCompression", 1.0).toDouble();
        data.agcEnabled = legacyPreset.value("agcEnabled", true).toBool();
        data.consoleType = legacyPreset.value("consoleType", "Eos").toString();
        data.lowSoloMode = legacyPreset.value("lowSoloMode", false).toBool();
        data.bpmActive = legacyPreset.value("bpm/Active", false).toBool();
        data.autoBpm = legacyPreset.value("autoBpm", false).toBool();
        data.waveformVisible = legacyPreset.value("waveformVisible", true).toBool();
        data.minBpm = legacyPreset.value("bpm/Min", 75).toInt();
        data.tapBpm = legacyPreset.value("bpm/tapvalue", 60).toInt();
        data.bpmMute = legacyPreset.value("bpm/mute", false).toBool();
        data.version = legacyPreset.value("version").toString();
        data.formatVersion = formatVersion;
        data.changedAt = legacyPreset.value("changedAt").toString();
        
        // BPM OSC commands
        QString bpmOnCommand = legacyPreset.value("bpm/on").toString();
        QString bpmOffCommand = legacyPreset.value("bpm/off").toString();
        if (!bpmOnCommand.isEmpty() || !bpmOffCommand.isEmpty()) {
            data.bpmOscCommands.append(bpmOnCommand);
            data.bpmOscCommands.append(bpmOffCommand);
        }
        
        // Save as JSON preset (keeping same filename)
        QString jsonPath = filePath;  // Keep .s2l extension, PresetManager will handle format
        if (presetManager->savePresetFile(jsonPath, data)) {
            successCount++;
            Logger::debug(QString("Migrated preset: %1").arg(fileInfo.fileName()));
        } else {
            Logger::warning(QString("Failed to migrate preset: %1").arg(fileInfo.fileName()));
        }
    }
    
    Logger::info(QString("Migrated %1 of %2 presets").arg(successCount).arg(files.size()));
    return successCount == files.size();
}

QString SettingsMigration::backupLegacySettings()
{
    QString legacyPath = legacySettingsPath();
    if (legacyPath.isEmpty() || !QFile::exists(legacyPath)) {
        return QString();
    }
    
    // Create backup filename with timestamp
    QFileInfo fileInfo(legacyPath);
    QString backupName = fileInfo.baseName() + 
                         QDateTime::currentDateTime().toString("_yyyyMMdd_HHmmss") +
                         ".ini.bak";
    QString backupPath = fileInfo.dir().filePath(backupName);
    
    if (QFile::copy(legacyPath, backupPath)) {
        return backupPath;
    }
    
    return QString();
}

QString SettingsMigration::legacySettingsPath()
{
    // QSettings with IniFormat stores in platform-specific locations:
    // Linux: ~/.config/ETC/Sound2Light.conf
    // Windows: %APPDATA%/ETC/Sound2Light.ini
    // macOS: ~/Library/Preferences/com.ETC.Sound2Light.plist (but we use INI)
    
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings legacySettings(AppInfo::legacyOrganizationName(),
                             AppInfo::legacyApplicationName());
    return legacySettings.fileName();
}

QString SettingsMigration::legacyPresetDirectory()
{
    // Standard data location for legacy app
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

} // namespace sound2osc
