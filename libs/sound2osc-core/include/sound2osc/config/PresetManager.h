// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
// SPDX-License-Identifier: MIT
//
// Preset management for sound2osc

#ifndef SOUND2OSC_CONFIG_PRESETMANAGER_H
#define SOUND2OSC_CONFIG_PRESETMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QVariant>

namespace sound2osc {

class IConfigStore;

/**
 * @brief Data structure for preset settings
 * 
 * Contains all values that make up a preset, independent of
 * the trigger generators and other components.
 */
struct PresetData {
    // FFT/DSP settings
    bool decibelConversion = false;
    double fftGain = 1.0;
    double fftCompression = 1.0;
    bool agcEnabled = true;
    
    // UI settings
    bool lowSoloMode = false;
    bool waveformVisible = true;
    QString consoleType = "Eos";
    
    // BPM settings
    bool bpmActive = false;
    bool autoBpm = false;
    int minBpm = 75;
    int tapBpm = 60;
    bool bpmMute = false;
    QStringList bpmOscCommands;
    
    // Metadata
    QString version;
    int formatVersion = 0;
    QString changedAt;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject toJson() const;
    
    /**
     * @brief Load from JSON representation
     * @param json JSON object containing preset data
     * @return true if successful
     */
    bool fromJson(const QJsonObject& json);
    
    /**
     * @brief Reset to default values
     */
    void reset();
};

/**
 * @brief Manages preset loading, saving, and listing
 * 
 * This class handles preset persistence and provides a clean API
 * for preset management. It does NOT directly modify trigger generators
 * or other components - that responsibility remains with MainController.
 * 
 * Thread-safe: All public methods can be called from any thread.
 */
class PresetManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentPresetName READ currentPresetName NOTIFY currentPresetChanged)
    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY unsavedChangesChanged)

public:
    /**
     * @brief Construct preset manager
     * @param presetDir Directory for preset files
     * @param parent QObject parent
     */
    explicit PresetManager(const QString& presetDir, QObject* parent = nullptr);
    ~PresetManager() override;

    // =========================================================================
    // Preset file operations
    // =========================================================================

    /**
     * @brief Load preset data from file
     * @param fileName Path to preset file (can have file:// prefix)
     * @return PresetData struct with loaded values, or default if failed
     */
    PresetData loadPresetFile(const QString& fileName);

    /**
     * @brief Save preset data to file
     * @param fileName Path to preset file
     * @param data Preset data to save
     * @param isAutosave If true, doesn't update current preset name
     * @return true if save succeeded
     */
    bool savePresetFile(const QString& fileName, const PresetData& data, bool isAutosave = false);

    /**
     * @brief Check if a preset file exists
     * @param fileName Path to preset file
     */
    bool presetExists(const QString& fileName) const;

    /**
     * @brief Delete a preset file
     * @param fileName Path to preset file
     * @return true if deletion succeeded
     */
    bool deletePreset(const QString& fileName);

    /**
     * @brief List all preset files in the preset directory
     * @return List of preset file paths
     */
    QStringList listPresets() const;

    // =========================================================================
    // Current preset state
    // =========================================================================

    /**
     * @brief Get the preset directory path
     */
    QString presetDirectory() const;

    /**
     * @brief Get current preset file name (full path)
     */
    QString currentPresetPath() const;

    /**
     * @brief Get current preset name (base name without extension)
     */
    QString currentPresetName() const;

    /**
     * @brief Set the current preset path
     * @param path Full path to preset file
     */
    void setCurrentPresetPath(const QString& path);

    /**
     * @brief Clear current preset (no preset loaded)
     */
    void clearCurrentPreset();

    /**
     * @brief Check if there are unsaved changes
     */
    bool hasUnsavedChanges() const;

    /**
     * @brief Mark preset as having unsaved changes
     */
    void markAsChanged();

    /**
     * @brief Mark preset as saved (no unsaved changes)
     */
    void markAsSaved();

    // =========================================================================
    // Autosave functionality
    // =========================================================================

    /**
     * @brief Get path to autosave file
     */
    QString autosavePath() const;

    /**
     * @brief Load autosave file
     * @return PresetData from autosave, or defaults if not found
     */
    PresetData loadAutosave();

    /**
     * @brief Save to autosave file
     * @param data Preset data to save
     */
    void saveAutosave(const PresetData& data);

    // =========================================================================
    // Utility methods
    // =========================================================================

    /**
     * @brief Clean up file path (remove file:// prefix, add extension)
     * @param rawPath Raw path from file dialog or OSC
     * @param addExtension Whether to add .s2l extension if missing
     * @return Cleaned path
     */
    static QString cleanFilePath(const QString& rawPath, bool addExtension = true);

    /**
     * @brief Check if settings format version is valid
     * @param formatVersion Version number from file
     * @return true if format is compatible
     */
    static bool isFormatValid(int formatVersion);

signals:
    /**
     * @brief Emitted when current preset changes
     */
    void currentPresetChanged();

    /**
     * @brief Emitted when unsaved changes state changes
     */
    void unsavedChangesChanged();

    /**
     * @brief Emitted when a preset is loaded
     * @param presetName Name of loaded preset
     */
    void presetLoaded(const QString& presetName);

    /**
     * @brief Emitted when a preset is saved
     * @param presetName Name of saved preset
     */
    void presetSaved(const QString& presetName);

    /**
     * @brief Emitted when preset load fails
     * @param error Error description
     */
    void loadError(const QString& error);

private:
    QString m_presetDir;
    QString m_currentPresetPath;
    bool m_hasUnsavedChanges = false;
};

} // namespace sound2osc

#endif // SOUND2OSC_CONFIG_PRESETMANAGER_H
