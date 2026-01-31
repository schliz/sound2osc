// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>

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

QJsonObject PresetManager::loadPresetFile(const QString& fileName)
{
    QJsonObject state;
    QString cleanPath = cleanFilePath(fileName, false);
    
    if (cleanPath.isEmpty()) {
        Logger::warning("Empty preset file path");
        emit loadError("Empty preset file path");
        return state;
    }
    
    QFile file(cleanPath);
    if (!file.exists()) {
        Logger::warning("Preset file does not exist: %1", cleanPath);
        emit loadError(QString("Preset does not exist: %1").arg(cleanPath));
        return state;
    }
    
    // Check if file is JSON
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
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
    
    QFile file(cleanPath);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::error("Failed to open file for writing: %1", cleanPath);
        return false;
    }
    
    QJsonDocument doc(finalState);
    file.write(doc.toJson());
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
    filters << "*.s2o" << "*.s2l";
    
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

QJsonObject PresetManager::loadAutosave()
{
    QString path = autosavePath();
    if (QFile::exists(path)) {
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
