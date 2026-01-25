// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
// SPDX-License-Identifier: MIT

#include <sound2osc/logging/Logger.h>

#include <QCoreApplication>
#include <QThread>
#include <iostream>

// Platform-specific includes for system logging
#ifdef Q_OS_LINUX
#include <syslog.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_MACOS
#include <os/log.h>
#endif

namespace sound2osc {

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::initialize(const QString& appName, int outputs)
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    
    logger.m_appName = appName;
    logger.m_outputs = outputs;
    logger.m_initialized = true;
    
    if (outputs & static_cast<int>(Output::System)) {
        logger.initSystemLogging(appName);
    }
}

void Logger::shutdown()
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    
    if (logger.m_logStream) {
        logger.m_logStream->flush();
        logger.m_logStream.reset();
    }
    
    if (logger.m_logFile) {
        logger.m_logFile->close();
        logger.m_logFile.reset();
    }
    
    if (logger.m_outputs & static_cast<int>(Output::System)) {
        logger.shutdownSystemLogging();
    }
    
    logger.m_initialized = false;
}

void Logger::setLogLevel(Level level)
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    logger.m_level = level;
}

Logger::Level Logger::getLogLevel()
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    return logger.m_level;
}

bool Logger::setLogFile(const QString& filePath)
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    
    // Close existing file if open
    if (logger.m_logStream) {
        logger.m_logStream->flush();
        logger.m_logStream.reset();
    }
    if (logger.m_logFile) {
        logger.m_logFile->close();
        logger.m_logFile.reset();
    }
    
    // Ensure parent directory exists
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            std::cerr << "Logger: Failed to create log directory: " 
                      << dir.absolutePath().toStdString() << std::endl;
            return false;
        }
    }
    
    // Open new log file
    logger.m_logFile = std::make_unique<QFile>(filePath);
    if (!logger.m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << "Logger: Failed to open log file: " 
                  << filePath.toStdString() << std::endl;
        logger.m_logFile.reset();
        return false;
    }
    
    logger.m_logStream = std::make_unique<QTextStream>(logger.m_logFile.get());
    logger.m_outputs |= static_cast<int>(Output::File);
    
    return true;
}

void Logger::setFormat(const QString& format)
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    logger.m_format = format;
}

void Logger::addHandler(LogHandler handler)
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    logger.m_handlers.append(handler);
}

void Logger::clearHandlers()
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);
    logger.m_handlers.clear();
}

void Logger::debug(const QString& message)
{
    instance().log(Level::Debug, message);
}

void Logger::info(const QString& message)
{
    instance().log(Level::Info, message);
}

void Logger::warning(const QString& message)
{
    instance().log(Level::Warning, message);
}

void Logger::error(const QString& message)
{
    instance().log(Level::Error, message);
}

void Logger::critical(const QString& message)
{
    instance().log(Level::Critical, message);
}

QString Logger::getDefaultLogDir()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + "/.config/sound2osc";
    }
    return configDir + "/logs";
}

QString Logger::levelToString(Level level)
{
    switch (level) {
        case Level::Debug:    return "DEBUG";
        case Level::Info:     return "INFO";
        case Level::Warning:  return "WARN";
        case Level::Error:    return "ERROR";
        case Level::Critical: return "CRIT";
    }
    return "UNKNOWN";
}

void Logger::log(Level level, const QString& message)
{
    QMutexLocker locker(&m_mutex);
    
    // Check level threshold
    if (level < m_level) {
        return;
    }
    
    QString formattedMessage = formatMessage(level, message);
    
    // Write to console
    if (m_outputs & static_cast<int>(Output::Console)) {
        writeToConsole(level, formattedMessage);
    }
    
    // Write to file
    if ((m_outputs & static_cast<int>(Output::File)) && m_logStream) {
        writeToFile(formattedMessage);
    }
    
    // Write to system log
    if (m_outputs & static_cast<int>(Output::System)) {
        writeToSystem(level, message);
    }
    
    // Call custom handlers
    for (const auto& handler : m_handlers) {
        handler(level, message);
    }
}

QString Logger::formatMessage(Level level, const QString& message)
{
    QString result = m_format;
    
    result.replace("%timestamp%", QDateTime::currentDateTime().toString(Qt::ISODate));
    result.replace("%level%", levelToString(level).leftJustified(5));
    result.replace("%message%", message);
    result.replace("%thread%", QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId())));
    
    return result;
}

void Logger::writeToConsole(Level level, const QString& formattedMessage)
{
    // Use stderr for warnings and above, stdout for info/debug
    if (level >= Level::Warning) {
        std::cerr << formattedMessage.toStdString() << std::endl;
    } else {
        std::cout << formattedMessage.toStdString() << std::endl;
    }
}

void Logger::writeToFile(const QString& formattedMessage)
{
    if (m_logStream) {
        *m_logStream << formattedMessage << "\n";
        m_logStream->flush();
    }
}

void Logger::writeToSystem(Level level, const QString& message)
{
#ifdef Q_OS_LINUX
    int priority;
    switch (level) {
        case Level::Debug:    priority = LOG_DEBUG; break;
        case Level::Info:     priority = LOG_INFO; break;
        case Level::Warning:  priority = LOG_WARNING; break;
        case Level::Error:    priority = LOG_ERR; break;
        case Level::Critical: priority = LOG_CRIT; break;
        default:              priority = LOG_INFO; break;
    }
    syslog(priority, "%s", message.toUtf8().constData());
#endif

#ifdef Q_OS_WIN
    // Write to Windows Event Log
    HANDLE hEventLog = RegisterEventSourceA(nullptr, m_appName.toUtf8().constData());
    if (hEventLog) {
        WORD eventType;
        switch (level) {
            case Level::Debug:
            case Level::Info:
                eventType = EVENTLOG_INFORMATION_TYPE;
                break;
            case Level::Warning:
                eventType = EVENTLOG_WARNING_TYPE;
                break;
            case Level::Error:
            case Level::Critical:
                eventType = EVENTLOG_ERROR_TYPE;
                break;
            default:
                eventType = EVENTLOG_INFORMATION_TYPE;
                break;
        }
        
        const char* messageStr = message.toUtf8().constData();
        ReportEventA(hEventLog, eventType, 0, 0, nullptr, 1, 0, &messageStr, nullptr);
        DeregisterEventSource(hEventLog);
    }
#endif

#ifdef Q_OS_MACOS
    os_log_type_t logType;
    switch (level) {
        case Level::Debug:    logType = OS_LOG_TYPE_DEBUG; break;
        case Level::Info:     logType = OS_LOG_TYPE_INFO; break;
        case Level::Warning:  logType = OS_LOG_TYPE_DEFAULT; break;
        case Level::Error:    logType = OS_LOG_TYPE_ERROR; break;
        case Level::Critical: logType = OS_LOG_TYPE_FAULT; break;
        default:              logType = OS_LOG_TYPE_DEFAULT; break;
    }
    os_log_with_type(OS_LOG_DEFAULT, logType, "%{public}s", message.toUtf8().constData());
#endif
}

void Logger::initSystemLogging([[maybe_unused]] const QString& appName)
{
#ifdef Q_OS_LINUX
    openlog(appName.toUtf8().constData(), LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
#endif
    // Windows and macOS don't need explicit initialization
}

void Logger::shutdownSystemLogging()
{
#ifdef Q_OS_LINUX
    closelog();
#endif
    // Windows and macOS don't need explicit shutdown
}

} // namespace sound2osc
