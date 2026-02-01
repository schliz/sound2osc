// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
// SPDX-License-Identifier: MIT

#include <sound2osc/config/JsonConfigStore.h>
#include <sound2osc/logging/Logger.h>

#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace sound2osc {

JsonConfigStore::JsonConfigStore(const QString& filePath)
    : m_filePath(filePath)
{
    ensureStructure();
}

JsonConfigStore::JsonConfigStore(const QString& appName, bool useDefaultPath)
    : m_filePath(useDefaultPath ? getDefaultConfigPath(appName) : QString())
{
    ensureStructure();
}

JsonConfigStore::~JsonConfigStore()
{
    if (m_dirty) {
        save();
    }
}

QString JsonConfigStore::getDefaultConfigPath(const QString& appName)
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        // Fallback for systems without standard paths
        configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/." + appName;
    }
    return configDir + "/config.json";
}

void JsonConfigStore::ensureStructure()
{
    if (!m_root.contains("settings")) {
        m_root["settings"] = QJsonObject();
    }
    if (!m_root.contains("presets")) {
        m_root["presets"] = QJsonObject();
    }
}

QVariant JsonConfigStore::getValue(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    
    QStringList parts = splitKey(key);
    if (parts.isEmpty()) {
        return defaultValue;
    }
    
    QJsonObject settings = m_root["settings"].toObject();
    
    if (parts.size() == 1) {
        // Simple key
        if (settings.contains(parts[0])) {
            return jsonToVariant(settings[parts[0]]);
        }
    } else if (parts.size() == 2) {
        // Group/key
        QJsonObject group = settings[parts[0]].toObject();
        if (group.contains(parts[1])) {
            return jsonToVariant(group[parts[1]]);
        }
    }
    
    return defaultValue;
}

void JsonConfigStore::setValue(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    
    QStringList parts = splitKey(key);
    if (parts.isEmpty()) {
        return;
    }
    
    QJsonObject settings = m_root["settings"].toObject();
    
    if (parts.size() == 1) {
        settings[parts[0]] = variantToJson(value);
    } else if (parts.size() == 2) {
        QJsonObject group = settings[parts[0]].toObject();
        group[parts[1]] = variantToJson(value);
        settings[parts[0]] = group;
    }
    
    m_root["settings"] = settings;
    m_dirty = true;
}

bool JsonConfigStore::contains(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    
    QStringList parts = splitKey(key);
    if (parts.isEmpty()) {
        return false;
    }
    
    QJsonObject settings = m_root["settings"].toObject();
    
    if (parts.size() == 1) {
        return settings.contains(parts[0]);
    } else if (parts.size() == 2) {
        QJsonObject group = settings[parts[0]].toObject();
        return group.contains(parts[1]);
    }
    
    return false;
}

void JsonConfigStore::remove(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    
    QStringList parts = splitKey(key);
    if (parts.isEmpty()) {
        return;
    }
    
    QJsonObject settings = m_root["settings"].toObject();
    
    if (parts.size() == 1) {
        settings.remove(parts[0]);
    } else if (parts.size() == 2) {
        QJsonObject group = settings[parts[0]].toObject();
        group.remove(parts[1]);
        settings[parts[0]] = group;
    }
    
    m_root["settings"] = settings;
    m_dirty = true;
}

QStringList JsonConfigStore::getGroupKeys(const QString& group) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject settings = m_root["settings"].toObject();
    QJsonObject groupObj = settings[group].toObject();
    
    return groupObj.keys();
}

QVariant JsonConfigStore::getGroupValue(const QString& group, 
                                        const QString& key,
                                        const QVariant& defaultValue) const
{
    return getValue(group + "/" + key, defaultValue);
}

void JsonConfigStore::setGroupValue(const QString& group, 
                                    const QString& key, 
                                    const QVariant& value)
{
    setValue(group + "/" + key, value);
}

bool JsonConfigStore::savePreset(const QString& presetName, const QJsonObject& presetData)
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject presets = m_root["presets"].toObject();
    presets[presetName] = presetData;
    m_root["presets"] = presets;
    m_dirty = true;
    
    Logger::debug("Preset saved: %1", presetName);
    return true;
}

QJsonObject JsonConfigStore::loadPreset(const QString& presetName) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject presets = m_root["presets"].toObject();
    if (presets.contains(presetName)) {
        Logger::debug("Preset loaded: %1", presetName);
        return presets[presetName].toObject();
    }
    
    Logger::warning("Preset not found: %1", presetName);
    return QJsonObject();
}

bool JsonConfigStore::presetExists(const QString& presetName) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject presets = m_root["presets"].toObject();
    return presets.contains(presetName);
}

bool JsonConfigStore::deletePreset(const QString& presetName)
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject presets = m_root["presets"].toObject();
    if (presets.contains(presetName)) {
        presets.remove(presetName);
        m_root["presets"] = presets;
        m_dirty = true;
        Logger::info("Preset deleted: %1", presetName);
        return true;
    }
    
    return false;
}

QStringList JsonConfigStore::listPresets() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject presets = m_root["presets"].toObject();
    return presets.keys();
}

bool JsonConfigStore::load()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_filePath.isEmpty()) {
        Logger::warning("JsonConfigStore: No file path specified");
        return false;
    }
    
    std::filesystem::path path(m_filePath.toStdString());
    
    if (!std::filesystem::exists(path)) {
        Logger::info("Config file does not exist, will create: %1", m_filePath);
        ensureStructure();
        m_dirty = true;
        return true;  // Not an error, will be created on save
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to open config file: %1", m_filePath);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    QByteArray data = QByteArray::fromStdString(content);
    
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        Logger::error("JSON parse error at %1: %2", 
                      parseError.offset, parseError.errorString());
        return false;
    }
    
    if (!doc.isObject()) {
        Logger::error("Config file root is not a JSON object");
        return false;
    }
    
    m_root = doc.object();
    ensureStructure();
    m_dirty = false;
    
    Logger::info("Configuration loaded from: %1", m_filePath);
    return true;
}

bool JsonConfigStore::save()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_filePath.isEmpty()) {
        Logger::warning("JsonConfigStore: No file path specified for save");
        return false;
    }
    
    std::filesystem::path path(m_filePath.toStdString());
    
    try {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::error("Failed to create config directory: %1", QString::fromStdString(e.what()));
        return false;
    }
    
    // Use atomic write (write to temp, then rename)
    std::filesystem::path tempPath = path;
    tempPath += ".tmp";
    
    std::ofstream file(tempPath);
    if (!file.is_open()) {
        Logger::error("Failed to open config file for writing: %1", m_filePath);
        return false;
    }
    
    QJsonDocument doc(m_root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    file << data.toStdString();
    
    if (file.fail()) {
        Logger::error("Failed to write config data");
        file.close();
        std::filesystem::remove(tempPath);
        return false;
    }
    
    file.close();
    
    try {
        std::filesystem::rename(tempPath, path);
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::error("Failed to commit config file: %1", QString::fromStdString(e.what()));
        return false;
    }
    
    m_dirty = false;
    Logger::debug("Configuration saved to: %1", m_filePath);
    return true;
}

void JsonConfigStore::sync()
{
    if (m_dirty) {
        save();
    }
}

bool JsonConfigStore::isDirty() const
{
    QMutexLocker locker(&m_mutex);
    return m_dirty;
}

QString JsonConfigStore::getStoragePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_filePath;
}

QString JsonConfigStore::getBackendType() const
{
    return "json";
}

QJsonObject JsonConfigStore::toJsonObject() const
{
    QMutexLocker locker(&m_mutex);
    return m_root;
}

void JsonConfigStore::fromJsonObject(const QJsonObject& root)
{
    QMutexLocker locker(&m_mutex);
    m_root = root;
    ensureStructure();
    m_dirty = true;
}

QJsonValue JsonConfigStore::variantToJson(const QVariant& value) const
{
    // Handle specific types explicitly
    switch (value.typeId()) {
        case QMetaType::Bool:
            return QJsonValue(value.toBool());
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            return QJsonValue(value.toLongLong());
        case QMetaType::Float:
        case QMetaType::Double:
            return QJsonValue(value.toDouble());
        case QMetaType::QString:
            return QJsonValue(value.toString());
        case QMetaType::QStringList: {
            QJsonArray arr;
            for (const QString& s : value.toStringList()) {
                arr.append(s);
            }
            return arr;
        }
        case QMetaType::QVariantList: {
            QJsonArray arr;
            for (const QVariant& v : value.toList()) {
                arr.append(variantToJson(v));
            }
            return arr;
        }
        case QMetaType::QVariantMap: {
            QJsonObject obj;
            QVariantMap map = value.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                obj[it.key()] = variantToJson(it.value());
            }
            return obj;
        }
        default:
            // Fallback: try to convert to string
            return QJsonValue(value.toString());
    }
}

QVariant JsonConfigStore::jsonToVariant(const QJsonValue& value) const
{
    switch (value.type()) {
        case QJsonValue::Bool:
            return QVariant(value.toBool());
        case QJsonValue::Double:
            return QVariant(value.toDouble());
        case QJsonValue::String:
            return QVariant(value.toString());
        case QJsonValue::Array: {
            QVariantList list;
            for (const QJsonValue& v : value.toArray()) {
                list.append(jsonToVariant(v));
            }
            return list;
        }
        case QJsonValue::Object: {
            QVariantMap map;
            QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                map[it.key()] = jsonToVariant(it.value());
            }
            return map;
        }
        case QJsonValue::Null:
        case QJsonValue::Undefined:
        default:
            return QVariant();
    }
}

QStringList JsonConfigStore::splitKey(const QString& key) const
{
    return key.split('/', Qt::SkipEmptyParts);
}

} // namespace sound2osc
