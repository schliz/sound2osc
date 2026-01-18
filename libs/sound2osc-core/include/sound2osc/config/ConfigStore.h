// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT
//
// Configuration storage abstraction for sound2osc

#ifndef SOUND2OSC_CONFIG_CONFIGSTORE_H
#define SOUND2OSC_CONFIG_CONFIGSTORE_H

#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QStringList>

#include <memory>

namespace sound2osc {

/**
 * @brief Abstract interface for configuration storage
 * 
 * Provides a backend-agnostic way to store and retrieve application settings.
 * Implementations can use JSON files, QSettings, databases, etc.
 * 
 * Design rationale:
 * - Uses QVariant for flexibility (supports all Qt types)
 * - Supports both flat keys and grouped/hierarchical settings
 * - Preset data is stored as JSON objects for web-ui compatibility
 * - Thread-safe implementations are required
 */
class IConfigStore
{
public:
    virtual ~IConfigStore() = default;

    // =========================================================================
    // Basic key-value access
    // =========================================================================

    /**
     * @brief Get a configuration value
     * @param key Setting key (e.g., "osc/ipAddress" or "ui/waveformVisible")
     * @param defaultValue Value to return if key doesn't exist
     * @return The stored value or defaultValue
     */
    virtual QVariant getValue(const QString& key, 
                              const QVariant& defaultValue = QVariant()) const = 0;

    /**
     * @brief Set a configuration value
     * @param key Setting key
     * @param value Value to store
     */
    virtual void setValue(const QString& key, const QVariant& value) = 0;

    /**
     * @brief Check if a key exists
     * @param key Setting key to check
     * @return true if the key exists
     */
    virtual bool contains(const QString& key) const = 0;

    /**
     * @brief Remove a key and its value
     * @param key Setting key to remove
     */
    virtual void remove(const QString& key) = 0;

    // =========================================================================
    // Group/section access (for hierarchical settings)
    // =========================================================================

    /**
     * @brief Get all keys within a group
     * @param group Group name (e.g., "osc" returns "ipAddress", "port", etc.)
     * @return List of keys in that group
     */
    virtual QStringList getGroupKeys(const QString& group) const = 0;

    /**
     * @brief Get a value within a group
     * @param group Group name
     * @param key Key within the group
     * @param defaultValue Default value if not found
     * @return The stored value or defaultValue
     */
    virtual QVariant getGroupValue(const QString& group, 
                                   const QString& key,
                                   const QVariant& defaultValue = QVariant()) const = 0;

    /**
     * @brief Set a value within a group
     * @param group Group name
     * @param key Key within the group
     * @param value Value to store
     */
    virtual void setGroupValue(const QString& group, 
                               const QString& key, 
                               const QVariant& value) = 0;

    // =========================================================================
    // Preset management (for preset files as JSON objects)
    // =========================================================================

    /**
     * @brief Save a preset as a JSON object
     * @param presetName Name/identifier for the preset
     * @param presetData Complete preset data as JSON
     * @return true if save succeeded
     */
    virtual bool savePreset(const QString& presetName, 
                            const QJsonObject& presetData) = 0;

    /**
     * @brief Load a preset by name
     * @param presetName Name/identifier of the preset
     * @return Preset data as JSON, empty object if not found
     */
    virtual QJsonObject loadPreset(const QString& presetName) const = 0;

    /**
     * @brief Check if a preset exists
     * @param presetName Name/identifier of the preset
     * @return true if preset exists
     */
    virtual bool presetExists(const QString& presetName) const = 0;

    /**
     * @brief Delete a preset
     * @param presetName Name/identifier of the preset
     * @return true if deletion succeeded
     */
    virtual bool deletePreset(const QString& presetName) = 0;

    /**
     * @brief List all available presets
     * @return List of preset names
     */
    virtual QStringList listPresets() const = 0;

    // =========================================================================
    // Persistence operations
    // =========================================================================

    /**
     * @brief Load configuration from storage
     * @return true if load succeeded (or if storage didn't exist yet)
     */
    virtual bool load() = 0;

    /**
     * @brief Save configuration to storage
     * @return true if save succeeded
     */
    virtual bool save() = 0;

    /**
     * @brief Sync any pending changes to storage immediately
     */
    virtual void sync() = 0;

    /**
     * @brief Check if there are unsaved changes
     * @return true if there are pending changes
     */
    virtual bool isDirty() const = 0;

    // =========================================================================
    // Metadata
    // =========================================================================

    /**
     * @brief Get the storage location/path
     * @return Path or description of storage location
     */
    virtual QString getStoragePath() const = 0;

    /**
     * @brief Get backend type name
     * @return Backend identifier (e.g., "json", "qsettings")
     */
    virtual QString getBackendType() const = 0;
};

/**
 * @brief Convenience type alias for config store pointers
 */
using ConfigStorePtr = std::shared_ptr<IConfigStore>;

} // namespace sound2osc

#endif // SOUND2OSC_CONFIG_CONFIGSTORE_H
