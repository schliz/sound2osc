// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT

#include <sound2osc/core/AppInfo.h>
#include <sound2osc/core/versionInfo.h>

#include <QStringList>

namespace sound2osc {

// Static member initialization with defaults
QString AppInfo::s_organizationName = QStringLiteral("sound2osc");
QString AppInfo::s_organizationDisplayName = QStringLiteral("Sound2OSC Project");
QString AppInfo::s_applicationName = QStringLiteral("sound2osc");
QString AppInfo::s_applicationDisplayName = QStringLiteral("Sound2OSC");
QString AppInfo::s_applicationDescription = QStringLiteral("Real-time audio analysis to OSC trigger events");
QString AppInfo::s_presetFileExtension = QStringLiteral("s2l");
QString AppInfo::s_autosaveFileExtension = QStringLiteral("ats");
QString AppInfo::s_configFileName = QStringLiteral("config.json");
QString AppInfo::s_defaultConsoleType = QStringLiteral("Eos");
QStringList AppInfo::s_supportedConsoleTypes = QStringList{
    QStringLiteral("Eos"),
    QStringLiteral("Cobalt 7.2"),
    QStringLiteral("Cobalt 7.1"),
    QStringLiteral("ColorSource")
};

// Organization info
QString AppInfo::organizationName()
{
    return s_organizationName;
}

void AppInfo::setOrganizationName(const QString& name)
{
    s_organizationName = name;
}

QString AppInfo::organizationDisplayName()
{
    return s_organizationDisplayName;
}

void AppInfo::setOrganizationDisplayName(const QString& name)
{
    s_organizationDisplayName = name;
}

// Application info
QString AppInfo::applicationName()
{
    return s_applicationName;
}

void AppInfo::setApplicationName(const QString& name)
{
    s_applicationName = name;
}

QString AppInfo::applicationDisplayName()
{
    return s_applicationDisplayName;
}

void AppInfo::setApplicationDisplayName(const QString& name)
{
    s_applicationDisplayName = name;
}

QString AppInfo::applicationVersion()
{
    return VERSION_STRING;
}

QString AppInfo::applicationDescription()
{
    return s_applicationDescription;
}

void AppInfo::setApplicationDescription(const QString& description)
{
    s_applicationDescription = description;
}

// File/path configuration
QString AppInfo::presetFileExtension()
{
    return s_presetFileExtension;
}

void AppInfo::setPresetFileExtension(const QString& extension)
{
    s_presetFileExtension = extension;
}

QString AppInfo::autosaveFileExtension()
{
    return s_autosaveFileExtension;
}

void AppInfo::setAutosaveFileExtension(const QString& extension)
{
    s_autosaveFileExtension = extension;
}

QString AppInfo::configFileName()
{
    return s_configFileName;
}

void AppInfo::setConfigFileName(const QString& name)
{
    s_configFileName = name;
}

// Console support
QStringList AppInfo::supportedConsoleTypes()
{
    return s_supportedConsoleTypes;
}

void AppInfo::setSupportedConsoleTypes(const QStringList& types)
{
    s_supportedConsoleTypes = types;
}

QString AppInfo::defaultConsoleType()
{
    return s_defaultConsoleType;
}

void AppInfo::setDefaultConsoleType(const QString& type)
{
    s_defaultConsoleType = type;
}

// Legacy support
QString AppInfo::legacyOrganizationName()
{
    return QStringLiteral("ETC");
}

QString AppInfo::legacyApplicationName()
{
    return QStringLiteral("Sound2Light");
}

// Utility
void AppInfo::resetToDefaults()
{
    s_organizationName = QStringLiteral("sound2osc");
    s_organizationDisplayName = QStringLiteral("Sound2OSC Project");
    s_applicationName = QStringLiteral("sound2osc");
    s_applicationDisplayName = QStringLiteral("Sound2OSC");
    s_applicationDescription = QStringLiteral("Real-time audio analysis to OSC trigger events");
    s_presetFileExtension = QStringLiteral("s2l");
    s_autosaveFileExtension = QStringLiteral("ats");
    s_configFileName = QStringLiteral("config.json");
    s_supportedConsoleTypes = QStringList{
        QStringLiteral("Eos"),
        QStringLiteral("Cobalt 7.2"),
        QStringLiteral("Cobalt 7.1"),
        QStringLiteral("ColorSource")
    };
    s_defaultConsoleType = QStringLiteral("Eos");
}

} // namespace sound2osc
