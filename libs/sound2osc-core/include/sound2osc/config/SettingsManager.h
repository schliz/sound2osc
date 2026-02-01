// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
// SPDX-License-Identifier: MIT
//
// Application settings management for sound2osc

#ifndef SOUND2OSC_CONFIG_SETTINGSMANAGER_H
#define SOUND2OSC_CONFIG_SETTINGSMANAGER_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QHostAddress>
#include <memory>

namespace sound2osc {

class IConfigStore;

/**
 * @brief Manages application settings (preset-independent)
 * 
 * This class handles settings that are not part of presets but are
 * application-wide configuration, such as:
 * - OSC network settings (IP, ports)
 * - Window geometry
 * - Input device selection
 * - OSC logging preferences
 * 
 * Thread-safe: All public methods can be called from any thread.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

    // OSC network settings
    Q_PROPERTY(QString oscIpAddress READ oscIpAddress WRITE setOscIpAddress NOTIFY oscIpAddressChanged)
    Q_PROPERTY(quint16 oscUdpTxPort READ oscUdpTxPort WRITE setOscUdpTxPort NOTIFY oscUdpTxPortChanged)
    Q_PROPERTY(quint16 oscUdpRxPort READ oscUdpRxPort WRITE setOscUdpRxPort NOTIFY oscUdpRxPortChanged)
    Q_PROPERTY(quint16 oscTcpPort READ oscTcpPort WRITE setOscTcpPort NOTIFY oscTcpPortChanged)
    Q_PROPERTY(bool oscEnabled READ oscEnabled WRITE setOscEnabled NOTIFY oscEnabledChanged)
    Q_PROPERTY(bool useTcp READ useTcp WRITE setUseTcp NOTIFY useTcpChanged)
    Q_PROPERTY(bool useOsc_1_1 READ useOsc_1_1 WRITE setUseOsc_1_1 NOTIFY useOsc_1_1Changed)
    Q_PROPERTY(bool oscInputEnabled READ oscInputEnabled WRITE setOscInputEnabled NOTIFY oscInputEnabledChanged)

public:
    /**
     * @brief Construct settings manager with config store backend
     * @param configStore Configuration storage backend (JSON, QSettings, etc.)
     * @param parent QObject parent
     */
    explicit SettingsManager(std::shared_ptr<IConfigStore> configStore, QObject* parent = nullptr);
    
    /**
     * @brief Construct settings manager using default QSettings backend
     * @param parent QObject parent
     */
    explicit SettingsManager(QObject* parent = nullptr);
    
    ~SettingsManager() override;

    // =========================================================================
    // OSC Network Settings
    // =========================================================================

    QString oscIpAddress() const;
    void setOscIpAddress(const QString& address);
    
    quint16 oscUdpTxPort() const;
    void setOscUdpTxPort(quint16 port);
    
    quint16 oscUdpRxPort() const;
    void setOscUdpRxPort(quint16 port);
    
    quint16 oscTcpPort() const;
    void setOscTcpPort(quint16 port);
    
    bool oscEnabled() const;
    void setOscEnabled(bool enabled);
    
    bool useTcp() const;
    void setUseTcp(bool use);
    
    bool useOsc_1_1() const;
    void setUseOsc_1_1(bool use);
    
    bool oscInputEnabled() const;
    void setOscInputEnabled(bool enabled);

    // =========================================================================
    // OSC Logging Settings
    // =========================================================================

    bool oscLogIncomingEnabled() const;
    void setOscLogIncomingEnabled(bool enabled);
    
    bool oscLogOutgoingEnabled() const;
    void setOscLogOutgoingEnabled(bool enabled);

    // =========================================================================
    // Window Settings
    // =========================================================================

    QRect windowGeometry() const;
    void setWindowGeometry(const QRect& geometry);
    
    bool windowMaximized() const;
    void setWindowMaximized(bool maximized);

    // =========================================================================
    // Input Device Settings
    // =========================================================================

    QString inputDeviceName() const;
    void setInputDeviceName(const QString& name);

    // =========================================================================
    // Persistence
    // =========================================================================

    /**
     * @brief Load all settings from storage
     * @return true if load succeeded
     */
    bool load();

    /**
     * @brief Save all settings to storage
     * @return true if save succeeded
     */
    bool save();

    /**
     * @brief Sync pending changes immediately
     */
    void sync();

    /**
     * @brief Check if settings format is valid
     * @return true if settings can be loaded
     */
    bool isValid() const;

signals:
    // OSC settings signals
    void oscIpAddressChanged();
    void oscUdpTxPortChanged();
    void oscUdpRxPortChanged();
    void oscTcpPortChanged();
    void oscEnabledChanged();
    void useTcpChanged();
    void useOsc_1_1Changed();
    void oscInputEnabledChanged();
    void oscLogSettingsChanged();
    
    // Window signals
    void windowGeometryChanged();
    void windowMaximizedChanged();
    
    // Input signals
    void inputDeviceNameChanged();
    
    // General
    void settingsChanged();

private:
    void initDefaults();
    void loadFromQSettings();
    void saveToQSettings();
    
    std::shared_ptr<IConfigStore> m_configStore;
    bool m_useQSettings = false;
    bool m_isValid = false;
    
    // Cached values
    QString m_oscIpAddress;
    quint16 m_oscUdpTxPort = 9000;
    quint16 m_oscUdpRxPort = 8000;
    quint16 m_oscTcpPort = 3032;
    bool m_oscEnabled = false;
    bool m_useTcp = false;
    bool m_useOsc_1_1 = false;
    bool m_oscInputEnabled = true;
    bool m_oscLogIncoming = true;
    bool m_oscLogOutgoing = true;
    
    QRect m_windowGeometry;
    bool m_windowMaximized = false;
    QString m_inputDeviceName;
};

} // namespace sound2osc

#endif // SOUND2OSC_CONFIG_SETTINGSMANAGER_H
