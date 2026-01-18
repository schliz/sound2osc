// Copyright (c) 2016-2026 Electronic Theatre Controls, Inc.
// SPDX-License-Identifier: MIT

#include <sound2osc/config/SettingsManager.h>
#include <sound2osc/config/ConfigStore.h>
#include <sound2osc/logging/Logger.h>
#include <sound2osc/core/versionInfo.h>

#include <QSettings>

namespace sound2osc {

SettingsManager::SettingsManager(std::shared_ptr<IConfigStore> configStore, QObject* parent)
    : QObject(parent)
    , m_configStore(configStore)
    , m_useQSettings(false)
{
    initDefaults();
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_useQSettings(true)
{
    initDefaults();
}

SettingsManager::~SettingsManager()
{
    save();
}

void SettingsManager::initDefaults()
{
    m_oscIpAddress = "127.0.0.1";
    m_oscUdpTxPort = 9000;
    m_oscUdpRxPort = 8000;
    m_oscTcpPort = 3032;
    m_oscEnabled = false;
    m_useTcp = false;
    m_useOsc_1_1 = false;
    m_oscInputEnabled = true;
    m_oscLogIncoming = true;
    m_oscLogOutgoing = true;
    m_windowMaximized = false;
}

// ============================================================================
// OSC Network Settings
// ============================================================================

QString SettingsManager::oscIpAddress() const
{
    return m_oscIpAddress;
}

void SettingsManager::setOscIpAddress(const QString& address)
{
    if (m_oscIpAddress != address) {
        m_oscIpAddress = address;
        emit oscIpAddressChanged();
        emit settingsChanged();
    }
}

quint16 SettingsManager::oscUdpTxPort() const
{
    return m_oscUdpTxPort;
}

void SettingsManager::setOscUdpTxPort(quint16 port)
{
    if (m_oscUdpTxPort != port) {
        m_oscUdpTxPort = port;
        emit oscUdpTxPortChanged();
        emit settingsChanged();
    }
}

quint16 SettingsManager::oscUdpRxPort() const
{
    return m_oscUdpRxPort;
}

void SettingsManager::setOscUdpRxPort(quint16 port)
{
    if (m_oscUdpRxPort != port) {
        m_oscUdpRxPort = port;
        emit oscUdpRxPortChanged();
        emit settingsChanged();
    }
}

quint16 SettingsManager::oscTcpPort() const
{
    return m_oscTcpPort;
}

void SettingsManager::setOscTcpPort(quint16 port)
{
    if (m_oscTcpPort != port) {
        m_oscTcpPort = port;
        emit oscTcpPortChanged();
        emit settingsChanged();
    }
}

bool SettingsManager::oscEnabled() const
{
    return m_oscEnabled;
}

void SettingsManager::setOscEnabled(bool enabled)
{
    if (m_oscEnabled != enabled) {
        m_oscEnabled = enabled;
        emit oscEnabledChanged();
        emit settingsChanged();
    }
}

bool SettingsManager::useTcp() const
{
    return m_useTcp;
}

void SettingsManager::setUseTcp(bool use)
{
    if (m_useTcp != use) {
        m_useTcp = use;
        emit useTcpChanged();
        emit settingsChanged();
    }
}

bool SettingsManager::useOsc_1_1() const
{
    return m_useOsc_1_1;
}

void SettingsManager::setUseOsc_1_1(bool use)
{
    if (m_useOsc_1_1 != use) {
        m_useOsc_1_1 = use;
        emit useOsc_1_1Changed();
        emit settingsChanged();
    }
}

bool SettingsManager::oscInputEnabled() const
{
    return m_oscInputEnabled;
}

void SettingsManager::setOscInputEnabled(bool enabled)
{
    if (m_oscInputEnabled != enabled) {
        m_oscInputEnabled = enabled;
        emit oscInputEnabledChanged();
        emit settingsChanged();
    }
}

// ============================================================================
// OSC Logging Settings
// ============================================================================

bool SettingsManager::oscLogIncomingEnabled() const
{
    return m_oscLogIncoming;
}

void SettingsManager::setOscLogIncomingEnabled(bool enabled)
{
    if (m_oscLogIncoming != enabled) {
        m_oscLogIncoming = enabled;
        emit oscLogSettingsChanged();
        emit settingsChanged();
    }
}

bool SettingsManager::oscLogOutgoingEnabled() const
{
    return m_oscLogOutgoing;
}

void SettingsManager::setOscLogOutgoingEnabled(bool enabled)
{
    if (m_oscLogOutgoing != enabled) {
        m_oscLogOutgoing = enabled;
        emit oscLogSettingsChanged();
        emit settingsChanged();
    }
}

// ============================================================================
// Window Settings
// ============================================================================

QRect SettingsManager::windowGeometry() const
{
    return m_windowGeometry;
}

void SettingsManager::setWindowGeometry(const QRect& geometry)
{
    if (m_windowGeometry != geometry) {
        m_windowGeometry = geometry;
        emit windowGeometryChanged();
    }
}

bool SettingsManager::windowMaximized() const
{
    return m_windowMaximized;
}

void SettingsManager::setWindowMaximized(bool maximized)
{
    if (m_windowMaximized != maximized) {
        m_windowMaximized = maximized;
        emit windowMaximizedChanged();
    }
}

// ============================================================================
// Input Device Settings
// ============================================================================

QString SettingsManager::inputDeviceName() const
{
    return m_inputDeviceName;
}

void SettingsManager::setInputDeviceName(const QString& name)
{
    if (m_inputDeviceName != name) {
        m_inputDeviceName = name;
        emit inputDeviceNameChanged();
    }
}

// ============================================================================
// Persistence
// ============================================================================

bool SettingsManager::load()
{
    if (m_useQSettings) {
        loadFromQSettings();
    } else if (m_configStore) {
        if (!m_configStore->load()) {
            Logger::warning("Failed to load config store");
            return false;
        }
        
        // Load from config store
        m_oscIpAddress = m_configStore->getValue("osc/ipAddress", "127.0.0.1").toString();
        m_oscUdpTxPort = static_cast<quint16>(m_configStore->getValue("osc/udpTxPort", 9000).toInt());
        m_oscUdpRxPort = static_cast<quint16>(m_configStore->getValue("osc/udpRxPort", 8000).toInt());
        m_oscTcpPort = static_cast<quint16>(m_configStore->getValue("osc/tcpPort", 3032).toInt());
        m_oscEnabled = m_configStore->getValue("osc/enabled", false).toBool();
        m_useTcp = m_configStore->getValue("osc/useTcp", false).toBool();
        m_useOsc_1_1 = m_configStore->getValue("osc/useOsc_1_1", false).toBool();
        m_oscInputEnabled = m_configStore->getValue("osc/inputEnabled", true).toBool();
        m_oscLogIncoming = m_configStore->getValue("osc/logIncoming", true).toBool();
        m_oscLogOutgoing = m_configStore->getValue("osc/logOutgoing", true).toBool();
        
        m_windowGeometry = m_configStore->getValue("window/geometry").toRect();
        m_windowMaximized = m_configStore->getValue("window/maximized", false).toBool();
        
        m_inputDeviceName = m_configStore->getValue("audio/inputDevice").toString();
        
        m_isValid = true;
    }
    
    Logger::info("Settings loaded");
    return m_isValid;
}

bool SettingsManager::save()
{
    if (m_useQSettings) {
        saveToQSettings();
        return true;
    } else if (m_configStore) {
        // Save to config store
        m_configStore->setValue("formatVersion", SETTINGS_FORMAT_VERSION);
        
        m_configStore->setValue("osc/ipAddress", m_oscIpAddress);
        m_configStore->setValue("osc/udpTxPort", m_oscUdpTxPort);
        m_configStore->setValue("osc/udpRxPort", m_oscUdpRxPort);
        m_configStore->setValue("osc/tcpPort", m_oscTcpPort);
        m_configStore->setValue("osc/enabled", m_oscEnabled);
        m_configStore->setValue("osc/useTcp", m_useTcp);
        m_configStore->setValue("osc/useOsc_1_1", m_useOsc_1_1);
        m_configStore->setValue("osc/inputEnabled", m_oscInputEnabled);
        m_configStore->setValue("osc/logIncoming", m_oscLogIncoming);
        m_configStore->setValue("osc/logOutgoing", m_oscLogOutgoing);
        
        m_configStore->setValue("window/geometry", m_windowGeometry);
        m_configStore->setValue("window/maximized", m_windowMaximized);
        
        m_configStore->setValue("audio/inputDevice", m_inputDeviceName);
        
        return m_configStore->save();
    }
    
    return false;
}

void SettingsManager::sync()
{
    if (m_configStore) {
        m_configStore->sync();
    }
}

bool SettingsManager::isValid() const
{
    return m_isValid;
}

void SettingsManager::loadFromQSettings()
{
    QSettings settings;
    
    // Check format version
    int formatVersion = settings.value("formatVersion").toInt();
    if (formatVersion == 0) {
        Logger::info("First start - no settings to load");
        m_isValid = false;
        return;
    }
    
    if (formatVersion < SETTINGS_FORMAT_VERSION) {
        Logger::warning("Old settings format version: %1", formatVersion);
    }
    
    m_oscIpAddress = settings.value("oscIpAddress", "127.0.0.1").toString();
    m_oscUdpTxPort = static_cast<quint16>(settings.value("oscTxPort", 9000).toInt());
    m_oscUdpRxPort = static_cast<quint16>(settings.value("oscRxPort", 8000).toInt());
    m_oscTcpPort = static_cast<quint16>(settings.value("oscTcpPort", 3032).toInt());
    m_oscEnabled = settings.value("oscIsEnabled", false).toBool();
    m_useTcp = settings.value("oscUseTcp", false).toBool();
    m_useOsc_1_1 = settings.value("oscUse_1_1", false).toBool();
    m_oscInputEnabled = settings.value("oscInputEnabled", true).toBool();
    
    if (settings.value("oscLogSettingsValid").toBool()) {
        m_oscLogIncoming = settings.value("oscLogIncomingIsEnabled", true).toBool();
        m_oscLogOutgoing = settings.value("oscLogOutgoingIsEnabled", true).toBool();
    }
    
    m_windowGeometry = settings.value("windowGeometry").toRect();
    m_windowMaximized = settings.value("maximized", false).toBool();
    
    m_inputDeviceName = settings.value("inputDeviceName").toString();
    
    m_isValid = true;
}

void SettingsManager::saveToQSettings()
{
    QSettings settings;
    
    settings.setValue("formatVersion", SETTINGS_FORMAT_VERSION);
    
    settings.setValue("oscIpAddress", m_oscIpAddress);
    settings.setValue("oscTxPort", m_oscUdpTxPort);
    settings.setValue("oscRxPort", m_oscUdpRxPort);
    settings.setValue("oscTcpPort", m_oscTcpPort);
    settings.setValue("oscIsEnabled", m_oscEnabled);
    settings.setValue("oscUseTcp", m_useTcp);
    settings.setValue("oscUse_1_1", m_useOsc_1_1);
    settings.setValue("oscInputEnabled", m_oscInputEnabled);
    
    settings.setValue("oscLogSettingsValid", true);
    settings.setValue("oscLogIncomingIsEnabled", m_oscLogIncoming);
    settings.setValue("oscLogOutgoingIsEnabled", m_oscLogOutgoing);
    
    settings.setValue("windowGeometry", m_windowGeometry);
    settings.setValue("maximized", m_windowMaximized);
    
    settings.setValue("inputDeviceName", m_inputDeviceName);
    
    Logger::debug("Settings saved to QSettings");
}

} // namespace sound2osc
