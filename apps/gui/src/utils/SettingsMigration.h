// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT
//
// Migration utility for QSettings (INI) to JSON config

#ifndef SOUND2OSC_UTILS_SETTINGSMIGRATION_H
#define SOUND2OSC_UTILS_SETTINGSMIGRATION_H

#include <QString>
#include <memory>

namespace sound2osc {

class SettingsManager;
class PresetManager;

/**
 * @brief Handles migration from legacy QSettings (INI) to new JSON config
 * 
 * This class provides one-time migration functionality to preserve user
 * settings when upgrading from older versions that used QSettings.
 * 
 * Migration process:
 * 1. Detect if legacy QSettings exist
 * 2. Backup legacy settings
 * 3. Migrate values to SettingsManager (app settings)
 * 4. Migrate preset files (.s2l) to JSON format via PresetManager
 * 5. Mark migration as complete
 * 
 * Usage in main.cpp:
 * @code
 * if (SettingsMigration::hasLegacySettings()) {
 *     SettingsMigration::migrate(settingsManager, presetManager);
 * }
 * @endcode
 */
class SettingsMigration
{
public:
    /**
     * @brief Check if legacy QSettings exist and need migration
     * @return true if legacy settings found and not yet migrated
     */
    static bool hasLegacySettings();

    /**
     * @brief Check if migration has already been completed
     * @return true if migration was previously completed
     */
    static bool migrationCompleted();

    /**
     * @brief Mark migration as completed (prevents re-migration)
     */
    static void markMigrationComplete();

    /**
     * @brief Perform full migration from QSettings to JSON
     * @param settingsManager Target for application settings
     * @param presetManager Target for preset data
     * @return true if migration succeeded
     */
    static bool migrate(SettingsManager* settingsManager, 
                        PresetManager* presetManager);

    /**
     * @brief Migrate only application settings (not presets)
     * @param settingsManager Target for settings
     * @return true if migration succeeded
     */
    static bool migrateSettings(SettingsManager* settingsManager);

    /**
     * @brief Migrate preset files from INI to JSON format
     * @param presetManager Target for preset data
     * @return true if migration succeeded
     */
    static bool migratePresets(PresetManager* presetManager);

    /**
     * @brief Backup legacy settings before migration
     * @return Path to backup file, empty on failure
     */
    static QString backupLegacySettings();

    /**
     * @brief Get path to legacy QSettings INI file
     * @return Path to INI file (platform-specific)
     */
    static QString legacySettingsPath();

    /**
     * @brief Get legacy preset directory path
     * @return Path to legacy preset directory
     */
    static QString legacyPresetDirectory();

private:
    SettingsMigration() = delete; // Static utility class
};

} // namespace sound2osc

#endif // SOUND2OSC_UTILS_SETTINGSMIGRATION_H
