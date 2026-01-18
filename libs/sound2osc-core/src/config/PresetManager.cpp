// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT

#include <sound2osc/config/PresetManager.h>
#include <sound2osc/logging/Logger.h>
#include <sound2osc/core/versionInfo.h>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>

namespace sound2osc {

// Current settings format version - increment when format changes
static constexpr int CURRENT_FORMAT_VERSION = SETTINGS_FORMAT_VERSION;

// ============================================================================
// PresetData implementation
// ============================================================================

QJsonObject PresetData::toJson() const
{
    QJsonObject json;
    
    // Metadata
    json["version"] = QString(VERSION_STRING);
    json["formatVersion"] = CURRENT_FORMAT_VERSION;
    json["changedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // FFT/DSP settings
    json["decibelConversion"] = decibelConversion;
    json["fftGain"] = fftGain;
    json["fftCompression"] = fftCompression;
    json["agcEnabled"] = agcEnabled;
    
    // UI settings
    json["lowSoloMode"] = lowSoloMode;
    json["waveformVisible"] = waveformVisible;
    json["consoleType"] = consoleType;
    
    // BPM settings
    QJsonObject bpmJson;
    bpmJson["active"] = bpmActive;
    bpmJson["auto"] = autoBpm;
    bpmJson["min"] = minBpm;
    bpmJson["tapValue"] = tapBpm;
    bpmJson["mute"] = bpmMute;
    
    QJsonArray commandsArray;
    for (const QString& cmd : bpmOscCommands) {
        commandsArray.append(cmd);
    }
    bpmJson["oscCommands"] = commandsArray;
    
    json["bpm"] = bpmJson;
    
    return json;
}

bool PresetData::fromJson(const QJsonObject& json)
{
    // Check format version
    formatVersion = json["formatVersion"].toInt(0);
    if (!PresetManager::isFormatValid(formatVersion)) {
        return false;
    }
    
    // Metadata
    version = json["version"].toString();
    changedAt = json["changedAt"].toString();
    
    // FFT/DSP settings
    decibelConversion = json["decibelConversion"].toBool(false);
    fftGain = json["fftGain"].toDouble(1.0);
    fftCompression = json["fftCompression"].toDouble(1.0);
    agcEnabled = json["agcEnabled"].toBool(true);
    
    // UI settings
    lowSoloMode = json["lowSoloMode"].toBool(false);
    waveformVisible = json["waveformVisible"].toBool(true);
    consoleType = json["consoleType"].toString("Eos");
    
    // BPM settings
    QJsonObject bpmJson = json["bpm"].toObject();
    bpmActive = bpmJson["active"].toBool(false);
    autoBpm = bpmJson["auto"].toBool(false);
    minBpm = bpmJson["min"].toInt(75);
    tapBpm = bpmJson["tapValue"].toInt(60);
    bpmMute = bpmJson["mute"].toBool(false);
    
    bpmOscCommands.clear();
    QJsonArray commandsArray = bpmJson["oscCommands"].toArray();
    for (const QJsonValue& val : commandsArray) {
        bpmOscCommands.append(val.toString());
    }
    
    return true;
}

void PresetData::reset()
{
    decibelConversion = false;
    fftGain = 1.0;
    fftCompression = 1.0;
    agcEnabled = true;
    lowSoloMode = false;
    waveformVisible = true;
    consoleType = "Eos";
    bpmActive = false;
    autoBpm = false;
    minBpm = 75;
    tapBpm = 60;
    bpmMute = false;
    bpmOscCommands.clear();
    version.clear();
    formatVersion = 0;
    changedAt.clear();
}

// ============================================================================
// PresetManager implementation
// ============================================================================

PresetManager::PresetManager(const QString& presetDir, QObject* parent)
    : QObject(parent)
    , m_presetDir(presetDir)
{
    // Ensure preset directory exists
    QDir dir(m_presetDir);
    if (!dir.exists()) {
        dir.mkpath(".");
        Logger::info("Created preset directory: %1", m_presetDir);
    }
}

PresetManager::~PresetManager() = default;

PresetData PresetManager::loadPresetFile(const QString& fileName)
{
    PresetData data;
    QString cleanPath = cleanFilePath(fileName, false);
    
    if (cleanPath.isEmpty()) {
        Logger::warning("Empty preset file path");
        emit loadError("Empty preset file path");
        return data;
    }
    
    QFile file(cleanPath);
    if (!file.exists()) {
        Logger::warning("Preset file does not exist: %1", cleanPath);
        emit loadError(QString("Preset does not exist: %1").arg(cleanPath));
        return data;
    }
    
    // Try to load as QSettings (INI format) for backward compatibility
    QSettings settings(cleanPath, QSettings::IniFormat);
    
    int formatVersion = settings.value("formatVersion").toInt();
    if (!isFormatValid(formatVersion)) {
        Logger::warning("Invalid preset format version: %1", formatVersion);
        emit loadError("Invalid preset format version");
        return data;
    }
    
    // Load from QSettings format (existing .s2l files)
    data.formatVersion = formatVersion;
    data.version = settings.value("version").toString();
    data.changedAt = settings.value("changedAt").toString();
    
    data.decibelConversion = settings.value("dbConversion").toBool();
    data.fftGain = settings.value("fftGain").toDouble();
    data.fftCompression = settings.value("fftCompression").toDouble();
    data.agcEnabled = settings.value("agcEnabled").toBool();
    
    data.lowSoloMode = settings.value("lowSoloMode").toBool();
    data.waveformVisible = settings.value("waveformVisible", true).toBool();
    data.consoleType = settings.value("consoleType").toString();
    
    data.bpmActive = settings.value("bpm/Active", false).toBool();
    data.autoBpm = settings.value("autoBpm", false).toBool();
    data.minBpm = settings.value("bpm/Min", 75).toInt();
    data.tapBpm = settings.value("bpm/tapvalue", 60).toInt();
    data.bpmMute = settings.value("bpm/mute", false).toBool();
    
    Logger::info("Preset loaded: %1", cleanPath);
    emit presetLoaded(QFileInfo(cleanPath).baseName());
    
    return data;
}

bool PresetManager::savePresetFile(const QString& fileName, const PresetData& data, bool isAutosave)
{
    QString cleanPath = cleanFilePath(fileName, !isAutosave);
    
    if (cleanPath.isEmpty()) {
        Logger::error("Empty preset file path for save");
        return false;
    }
    
    // Save as QSettings (INI format) for backward compatibility
    QSettings settings(cleanPath, QSettings::IniFormat);
    
    settings.setValue("version", VERSION_STRING);
    settings.setValue("formatVersion", CURRENT_FORMAT_VERSION);
    settings.setValue("changedAt", QDateTime::currentDateTime().toString());
    
    settings.setValue("dbConversion", data.decibelConversion);
    settings.setValue("fftGain", data.fftGain);
    settings.setValue("fftCompression", data.fftCompression);
    settings.setValue("agcEnabled", data.agcEnabled);
    
    settings.setValue("lowSoloMode", data.lowSoloMode);
    settings.setValue("waveformVisible", data.waveformVisible);
    settings.setValue("consoleType", data.consoleType);
    
    settings.setValue("bpm/Active", data.bpmActive);
    settings.setValue("autoBpm", data.autoBpm);
    settings.setValue("bpm/Min", data.minBpm);
    settings.setValue("bpm/tapvalue", data.tapBpm);
    settings.setValue("bpm/mute", data.bpmMute);
    
    settings.sync();
    
    if (settings.status() != QSettings::NoError) {
        Logger::error("Failed to save preset: %1", cleanPath);
        return false;
    }
    
    if (!isAutosave) {
        setCurrentPresetPath(cleanPath);
        markAsSaved();
        Logger::info("Preset saved: %1", cleanPath);
        emit presetSaved(QFileInfo(cleanPath).baseName());
    } else {
        Logger::debug("Autosave completed: %1", cleanPath);
    }
    
    return true;
}

bool PresetManager::presetExists(const QString& fileName) const
{
    QString cleanPath = cleanFilePath(fileName, false);
    return QFile::exists(cleanPath);
}

bool PresetManager::deletePreset(const QString& fileName)
{
    QString cleanPath = cleanFilePath(fileName, false);
    
    QFile file(cleanPath);
    if (!file.exists()) {
        return false;
    }
    
    if (file.remove()) {
        Logger::info("Preset deleted: %1", cleanPath);
        
        // Clear current preset if it was the deleted one
        if (m_currentPresetPath == cleanPath) {
            clearCurrentPreset();
        }
        
        return true;
    }
    
    Logger::error("Failed to delete preset: %1", cleanPath);
    return false;
}

QStringList PresetManager::listPresets() const
{
    QDir dir(m_presetDir);
    QStringList filters;
    filters << "*.s2l";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    
    QStringList presets;
    for (const QFileInfo& file : files) {
        presets.append(file.absoluteFilePath());
    }
    
    return presets;
}

QString PresetManager::presetDirectory() const
{
    return m_presetDir;
}

QString PresetManager::currentPresetPath() const
{
    return m_currentPresetPath;
}

QString PresetManager::currentPresetName() const
{
    if (m_currentPresetPath.isEmpty()) {
        return QString();
    }
    return QFileInfo(m_currentPresetPath).baseName();
}

void PresetManager::setCurrentPresetPath(const QString& path)
{
    if (m_currentPresetPath != path) {
        m_currentPresetPath = path;
        emit currentPresetChanged();
    }
}

void PresetManager::clearCurrentPreset()
{
    if (!m_currentPresetPath.isEmpty()) {
        m_currentPresetPath.clear();
        m_hasUnsavedChanges = false;
        emit currentPresetChanged();
        emit unsavedChangesChanged();
    }
}

bool PresetManager::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}

void PresetManager::markAsChanged()
{
    if (!m_hasUnsavedChanges) {
        m_hasUnsavedChanges = true;
        emit unsavedChangesChanged();
    }
}

void PresetManager::markAsSaved()
{
    if (m_hasUnsavedChanges) {
        m_hasUnsavedChanges = false;
        emit unsavedChangesChanged();
    }
}

QString PresetManager::autosavePath() const
{
    return m_presetDir + "/autosave.ats";
}

PresetData PresetManager::loadAutosave()
{
    QString path = autosavePath();
    if (QFile::exists(path)) {
        return loadPresetFile(path);
    }
    return PresetData();
}

void PresetManager::saveAutosave(const PresetData& data)
{
    savePresetFile(autosavePath(), data, true);
}

QString PresetManager::cleanFilePath(const QString& rawPath, bool addExtension)
{
    QString path = rawPath;
    
    // Remove file:// prefix from file dialogs
    if (path.startsWith("file:///")) {
        path = path.mid(8);
    } else if (path.startsWith("file://")) {
        path = path.mid(7);
    }
    
    // Add .s2l extension if needed
    if (addExtension && !path.toLower().endsWith(".s2l")) {
        path += ".s2l";
    }
    
    return path;
}

bool PresetManager::isFormatValid(int formatVersion)
{
    if (formatVersion == 0) {
        // First start, no data to restore
        return false;
    }
    if (formatVersion < CURRENT_FORMAT_VERSION) {
        // Old format, might need migration
        // For now, accept any non-zero version
        return true;
    }
    return true;
}

} // namespace sound2osc
