// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
//
// Application branding and metadata for sound2osc

#ifndef SOUND2OSC_CORE_APPINFO_H
#define SOUND2OSC_CORE_APPINFO_H

#include <QString>
#include <QStringList>

namespace sound2osc {

/**
 * @brief Centralized application branding and metadata
 * 
 * This class provides a single point of configuration for application
 * identity, making it easy to rebrand forks or customize deployments.
 * 
 * Default values can be overridden at runtime before QApplication is created.
 * 
 * Usage:
 * @code
 * // In main.cpp, before QApplication:
 * AppInfo::setOrganizationName("MyOrg");
 * AppInfo::setApplicationName("MyApp");
 * 
 * QCoreApplication::setOrganizationName(AppInfo::organizationName());
 * QCoreApplication::setApplicationName(AppInfo::applicationName());
 * @endcode
 */
class AppInfo
{
public:
    // =========================================================================
    // Organization Info
    // =========================================================================

    /**
     * @brief Get organization name (used for QSettings paths)
     * @return Organization name (default: "sound2osc")
     */
    static QString organizationName();

    /**
     * @brief Set organization name
     * @param name New organization name
     */
    static void setOrganizationName(const QString& name);

    /**
     * @brief Get organization display name (for UI)
     * @return Full organization name for display
     */
    static QString organizationDisplayName();

    /**
     * @brief Set organization display name
     * @param name Display name
     */
    static void setOrganizationDisplayName(const QString& name);

    // =========================================================================
    // Application Info
    // =========================================================================

    /**
     * @brief Get application name (used for QSettings paths)
     * @return Application name (default: "sound2osc")
     */
    static QString applicationName();

    /**
     * @brief Set application name
     * @param name New application name
     */
    static void setApplicationName(const QString& name);

    /**
     * @brief Get application display name (for window titles, etc.)
     * @return Display name (default: "sound2osc")
     */
    static QString applicationDisplayName();

    /**
     * @brief Set application display name
     * @param name Display name
     */
    static void setApplicationDisplayName(const QString& name);

    /**
     * @brief Get application version string
     * @return Version string (from versionInfo.h)
     */
    static QString applicationVersion();

    /**
     * @brief Get application description
     * @return Short description
     */
    static QString applicationDescription();

    /**
     * @brief Set application description
     * @param description Description text
     */
    static void setApplicationDescription(const QString& description);

    // =========================================================================
    // File/Path Configuration
    // =========================================================================

    /**
     * @brief Get preset file extension (without dot)
     * @return File extension (default: "s2o")
     */
    static QString presetFileExtension();

    /**
     * @brief Set preset file extension
     * @param extension Extension without dot (e.g., "s2l")
     */
    static void setPresetFileExtension(const QString& extension);

    /**
     * @brief Get autosave file extension
     * @return Autosave extension (default: "ats")
     */
    static QString autosaveFileExtension();

    /**
     * @brief Set autosave file extension
     * @param extension Extension without dot
     */
    static void setAutosaveFileExtension(const QString& extension);

    /**
     * @brief Get config file name (JSON configuration)
     * @return Config filename (default: "config.json")
     */
    static QString configFileName();

    /**
     * @brief Set config file name
     * @param name Config filename
     */
    static void setConfigFileName(const QString& name);

    // =========================================================================
    // Console/Integration Support
    // =========================================================================

    /**
     * @brief Get list of supported console types
     * @return List of console type identifiers
     */
    static QStringList supportedConsoleTypes();

    /**
     * @brief Set supported console types
     * @param types List of console type identifiers
     */
    static void setSupportedConsoleTypes(const QStringList& types);

    /**
     * @brief Get default console type
     * @return Default console type (default: "Eos")
     */
    static QString defaultConsoleType();

    /**
     * @brief Set default console type
     * @param type Default console type
     */
    static void setDefaultConsoleType(const QString& type);

    // =========================================================================
    // Legacy Support
    // =========================================================================

    /**
     * @brief Get legacy organization name (for migration from old settings)
     * @return Legacy organization name
     */
    static QString legacyOrganizationName();

    /**
     * @brief Get legacy application name (for migration)
     * @return Legacy application name
     */
    static QString legacyApplicationName();

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * @brief Reset all values to defaults
     */
    static void resetToDefaults();

private:
    // Singleton data storage
    static QString s_organizationName;
    static QString s_organizationDisplayName;
    static QString s_applicationName;
    static QString s_applicationDisplayName;
    static QString s_applicationDescription;
    static QString s_presetFileExtension;
    static QString s_autosaveFileExtension;
    static QString s_configFileName;
    static QStringList s_supportedConsoleTypes;
    static QString s_defaultConsoleType;
};

} // namespace sound2osc

#endif // SOUND2OSC_CORE_APPINFO_H
