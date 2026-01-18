// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT
//
// JSON file-based configuration storage for sound2osc

#ifndef SOUND2OSC_CONFIG_JSONCONFIGSTORE_H
#define SOUND2OSC_CONFIG_JSONCONFIGSTORE_H

#include <sound2osc/config/ConfigStore.h>

#include <QMutex>
#include <QJsonDocument>

namespace sound2osc {

/**
 * @brief JSON file-based implementation of IConfigStore
 * 
 * Stores configuration in a human-readable JSON file.
 * 
 * File structure:
 * {
 *   "settings": {
 *     "osc": { "ipAddress": "...", "port": 9000 },
 *     "ui": { "waveformVisible": true }
 *   },
 *   "presets": {
 *     "preset1": { ... },
 *     "preset2": { ... }
 *   }
 * }
 * 
 * Thread-safe: All public methods are protected by mutex.
 * Atomic writes: Uses write-to-temp-then-rename pattern.
 */
class JsonConfigStore : public IConfigStore
{
public:
    /**
     * @brief Construct with explicit file path
     * @param filePath Full path to JSON config file
     */
    explicit JsonConfigStore(const QString& filePath);

    /**
     * @brief Construct using default platform-specific path
     * @param appName Application name for path construction
     * 
     * Default paths:
     * - Linux: ~/.config/sound2osc/config.json
     * - Windows: %APPDATA%/sound2osc/config.json
     * - macOS: ~/Library/Application Support/sound2osc/config.json
     */
    explicit JsonConfigStore(const QString& appName, bool useDefaultPath);

    ~JsonConfigStore() override;

    // IConfigStore interface implementation
    QVariant getValue(const QString& key, 
                      const QVariant& defaultValue = QVariant()) const override;
    void setValue(const QString& key, const QVariant& value) override;
    bool contains(const QString& key) const override;
    void remove(const QString& key) override;

    QStringList getGroupKeys(const QString& group) const override;
    QVariant getGroupValue(const QString& group, 
                           const QString& key,
                           const QVariant& defaultValue = QVariant()) const override;
    void setGroupValue(const QString& group, 
                       const QString& key, 
                       const QVariant& value) override;

    bool savePreset(const QString& presetName, 
                    const QJsonObject& presetData) override;
    QJsonObject loadPreset(const QString& presetName) const override;
    bool presetExists(const QString& presetName) const override;
    bool deletePreset(const QString& presetName) override;
    QStringList listPresets() const override;

    bool load() override;
    bool save() override;
    void sync() override;
    bool isDirty() const override;

    QString getStoragePath() const override;
    QString getBackendType() const override;

    // JsonConfigStore-specific methods

    /**
     * @brief Get the entire configuration as a JSON object
     * @return Root JSON object
     */
    QJsonObject toJsonObject() const;

    /**
     * @brief Replace entire configuration from JSON object
     * @param root New root configuration
     */
    void fromJsonObject(const QJsonObject& root);

    /**
     * @brief Get default config file path for this platform
     * @param appName Application name
     * @return Platform-specific default path
     */
    static QString getDefaultConfigPath(const QString& appName = "sound2osc");

private:
    void ensureStructure();
    QJsonValue variantToJson(const QVariant& value) const;
    QVariant jsonToVariant(const QJsonValue& value) const;
    QStringList splitKey(const QString& key) const;

    mutable QMutex m_mutex;
    QString m_filePath;
    QJsonObject m_root;
    bool m_dirty = false;
};

} // namespace sound2osc

#endif // SOUND2OSC_CONFIG_JSONCONFIGSTORE_H
