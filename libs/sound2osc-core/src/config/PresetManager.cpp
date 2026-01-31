// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>

#include <sound2osc/config/PresetManager.h>
#include <sound2osc/logging/Logger.h>
#include <sound2osc/core/versionInfo.h>

#include <QFileInfo>
#include <QSettings>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace sound2osc {

// Current settings format version - increment when format changes
static constexpr int CURRENT_FORMAT_VERSION = SETTINGS_FORMAT_VERSION;

// ============================================================================
// PresetManager implementation
// ============================================================================

PresetManager::PresetManager(const QString& presetDir, QObject* parent)
    : QObject(parent)
    , m_presetDir(presetDir)
{
    // Ensure preset directory exists
    std::filesystem::path dir(m_presetDir.toStdString());
    try {
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
            Logger::info("Created preset directory: %1", m_presetDir);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::error("Failed to create preset directory: %1", QString::fromStdString(e.what()));
    }
}

PresetManager::~PresetManager() = default;

QJsonObject PresetManager::loadPresetFile(const QString& fileName)
{
    QJsonObject state;
    QString cleanPath = cleanFilePath(fileName, false);
    
    if (cleanPath.isEmpty()) {
        Logger::warning("Empty preset file path");
        emit loadError("Empty preset file path");
        return state;
    }
    
    std::filesystem::path path(cleanPath.toStdString());
    if (!std::filesystem::exists(path)) {
        Logger::warning("Preset file does not exist: %1", cleanPath);
        emit loadError(QString("Preset does not exist: %1").arg(cleanPath));
        return state;
    }
    
    // Check if file is JSON
    std::ifstream file(path);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        QByteArray data = QByteArray::fromStdString(content);
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            Logger::info("Loaded JSON preset: %1", cleanPath);
            emit presetLoaded(QFileInfo(cleanPath).baseName());
            return doc.object();
        }
    }
    
    // Fallback: Try to load as QSettings (INI format) for backward compatibility
    Logger::info("Attempting to load legacy preset: %1", cleanPath);
    state = convertLegacySettingsToJson(cleanPath);
    
    if (!state.isEmpty()) {
        Logger::info("Loaded legacy preset: %1", cleanPath);
        emit presetLoaded(QFileInfo(cleanPath).baseName());
    } else {
        Logger::error("Failed to load preset: %1", cleanPath);
        emit loadError("Failed to load preset");
    }
    
    return state;
}

bool PresetManager::savePresetFile(const QString& fileName, const QJsonObject& state, bool isAutosave)
{
    QString cleanPath = cleanFilePath(fileName, !isAutosave);
    
    if (cleanPath.isEmpty()) {
        Logger::error("Empty preset file path for save");
        return false;
    }
    
    // Inject metadata
    QJsonObject finalState = state;
    finalState["version"] = QString(VERSION_STRING);
    finalState["formatVersion"] = CURRENT_FORMAT_VERSION;
    finalState["changedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    std::filesystem::path path(cleanPath.toStdString());
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to open file for writing: %1", cleanPath);
        return false;
    }
    
    QJsonDocument doc(finalState);
    file << doc.toJson().toStdString();
    file.close();
    
    if (!isAutosave) {
        setCurrentPresetPath(cleanPath);
        markAsSaved();
        Logger::info("Preset saved (JSON): %1", cleanPath);
        emit presetSaved(QFileInfo(cleanPath).baseName());
    } else {
        Logger::debug("Autosave completed: %1", cleanPath);
    }
    
    return true;
}

// Helper to convert old INI presets to new JSON structure on the fly
QJsonObject PresetManager::convertLegacySettingsToJson(const QString& path)
{
    QSettings settings(path, QSettings::IniFormat);
    
    int formatVersion = settings.value("formatVersion").toInt();
    if (!isFormatValid(formatVersion)) {
        return QJsonObject();
    }
    
    QJsonObject state;
    
    // Global Settings
    state["lowSoloMode"] = settings.value("lowSoloMode").toBool();
    
    // DSP
    QJsonObject dsp;
    dsp["decibel"] = settings.value("dbConversion").toBool();
    dsp["gain"] = settings.value("fftGain").toDouble();
    dsp["compression"] = settings.value("fftCompression").toDouble();
    dsp["agc"] = settings.value("agcEnabled").toBool();
    state["dsp"] = dsp;
    
    // BPM
    QJsonObject bpm;
    bpm["active"] = settings.value("bpm/Active", false).toBool();
    bpm["auto"] = settings.value("autoBpm", false).toBool();
    bpm["min"] = settings.value("bpm/Min", 75).toInt();
    bpm["mute"] = settings.value("bpm/mute", false).toBool();
    
    QJsonObject bpmOsc;
    QJsonArray commands;
    int count = settings.value("bpm/osc/count").toInt();
    for (int i = 0; i < count; ++i) {
        commands.append(settings.value("bpm/osc/" + QString::number(i)).toString());
    }
    bpmOsc["commands"] = commands;
    bpm["osc"] = bpmOsc;
    state["bpm"] = bpm;
    
    // Triggers
    QJsonObject triggers;
    QStringList names = {"bass", "loMid", "hiMid", "high", "envelope", "silence"};
    
    for (const QString& name : names) {
        QJsonObject trigger;
        trigger["mute"] = settings.value(name + "/mute").toBool();
        trigger["threshold"] = settings.value(name + "/threshold").toDouble();
        trigger["midFreq"] = settings.value(name + "/midFreq").toInt();
        trigger["width"] = settings.value(name + "/width").toDouble();
        
        QJsonObject filter;
        filter["onDelay"] = settings.value(name + "/onDelay").toDouble();
        filter["offDelay"] = settings.value(name + "/offDelay").toDouble();
        filter["maxHold"] = settings.value(name + "/maxHold").toDouble();
        trigger["filter"] = filter;
        
        QJsonObject osc;
        osc["onMessage"] = settings.value(name + "/osc/onMessage").toString();
        osc["offMessage"] = settings.value(name + "/osc/offMessage").toString();
        osc["levelMessage"] = settings.value(name + "/osc/levelMessage").toString();
        osc["minLevelValue"] = settings.value(name + "/osc/minLevelValue").toDouble();
        osc["maxLevelValue"] = settings.value(name + "/osc/maxLevelValue").toDouble();
        osc["labelText"] = settings.value(name + "/osc/labelText").toString();
        trigger["osc"] = osc;
        
        triggers[name] = trigger;
    }
    state["triggers"] = triggers;
    
    return state;
}

bool PresetManager::presetExists(const QString& fileName) const
{
    QString cleanPath = cleanFilePath(fileName, false);
    return std::filesystem::exists(cleanPath.toStdString());
}

bool PresetManager::deletePreset(const QString& fileName)
{
    QString cleanPath = cleanFilePath(fileName, false);
    std::filesystem::path path(cleanPath.toStdString());
    
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    try {
        if (std::filesystem::remove(path)) {
            Logger::info("Preset deleted: %1", cleanPath);
            
            // Clear current preset if it was the deleted one
            if (m_currentPresetPath == cleanPath) {
                clearCurrentPreset();
            }
            
            return true;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::error("Failed to delete preset: %1. Error: %2", cleanPath, QString::fromStdString(e.what()));
    }
    
    Logger::error("Failed to delete preset: %1", cleanPath);
    return false;
}

QStringList PresetManager::listPresets() const
{
    QStringList presets;
    std::filesystem::path dir(m_presetDir.toStdString());
    
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        return presets;
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".s2o" || ext == ".s2l") {
                    presets.append(QString::fromStdString(entry.path().string()));
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::error("Failed to list presets: %1", QString::fromStdString(e.what()));
    }
    
    std::sort(presets.begin(), presets.end());
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

QJsonObject PresetManager::loadAutosave()
{
    QString path = autosavePath();
    if (std::filesystem::exists(path.toStdString())) {
        return loadPresetFile(path);
    }
    return QJsonObject();
}

void PresetManager::saveAutosave(const QJsonObject& state)
{
    savePresetFile(autosavePath(), state, true);
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
    
    // Add extension if needed
    if (addExtension) {
        if (!path.toLower().endsWith(".s2o") && !path.toLower().endsWith(".s2l")) {
            path += ".s2o";
        }
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
