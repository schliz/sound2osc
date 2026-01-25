// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
// SPDX-License-Identifier: MIT
//
// Cross-platform structured logging system for sound2osc

#ifndef SOUND2OSC_LOGGING_LOGGER_H
#define SOUND2OSC_LOGGING_LOGGER_H

#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

#include <functional>
#include <memory>

namespace sound2osc {

/**
 * @brief Cross-platform structured logging system
 * 
 * Thread-safe logger with support for multiple output targets:
 * - Console (stdout/stderr)
 * - File logging with rotation
 * - Platform-specific system logging (syslog/Event Log)
 * 
 * Usage:
 *   Logger::info("Application started");
 *   Logger::warning("Device not found: %1", deviceName);
 *   Logger::error("Connection failed: %1", errorMessage);
 */
class Logger
{
public:
    /**
     * @brief Log severity levels
     */
    enum class Level {
        Debug = 0,   ///< Detailed debug information
        Info,        ///< General information
        Warning,     ///< Warnings (recoverable issues)
        Error,       ///< Errors (non-fatal)
        Critical     ///< Critical errors (may cause crash)
    };

    /**
     * @brief Output targets for log messages
     */
    enum class Output {
        Console = 1 << 0,    ///< stdout/stderr
        File    = 1 << 1,    ///< Log file
        System  = 1 << 2     ///< syslog (Linux), Event Log (Windows), unified logging (macOS)
    };

    /**
     * @brief Custom log handler callback type
     */
    using LogHandler = std::function<void(Level level, const QString& message)>;

    // Disable copy and move
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief Initialize the logging system
     * @param appName Application name for system logs
     * @param outputs Bitwise OR of Output flags
     */
    static void initialize(const QString& appName = "sound2osc",
                          int outputs = static_cast<int>(Output::Console));

    /**
     * @brief Shutdown logging system and flush pending messages
     */
    static void shutdown();

    /**
     * @brief Set minimum log level (messages below this level are ignored)
     * @param level Minimum severity level
     */
    static void setLogLevel(Level level);

    /**
     * @brief Get current minimum log level
     */
    static Level getLogLevel();

    /**
     * @brief Set log file path
     * @param filePath Path to log file (parent directory must exist)
     * @return true if file was opened successfully
     */
    static bool setLogFile(const QString& filePath);

    /**
     * @brief Set log message format
     * @param format Format string with placeholders:
     *               %timestamp% - ISO timestamp
     *               %level%     - Log level name
     *               %message%   - Log message
     *               %thread%    - Thread ID
     * Default: "[%timestamp%] [%level%] %message%"
     */
    static void setFormat(const QString& format);

    /**
     * @brief Add custom log handler
     * @param handler Callback function for log messages
     */
    static void addHandler(LogHandler handler);

    /**
     * @brief Clear all custom handlers
     */
    static void clearHandlers();

    // Logging methods
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
    static void critical(const QString& message);

    // Logging methods with format arguments (up to 4 args)
    template<typename T1>
    static void debug(const QString& format, const T1& arg1) {
        debug(QString(format).arg(arg1));
    }
    template<typename T1, typename T2>
    static void debug(const QString& format, const T1& arg1, const T2& arg2) {
        debug(QString(format).arg(arg1).arg(arg2));
    }
    template<typename T1, typename T2, typename T3>
    static void debug(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3) {
        debug(QString(format).arg(arg1).arg(arg2).arg(arg3));
    }
    template<typename T1, typename T2, typename T3, typename T4>
    static void debug(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        debug(QString(format).arg(arg1).arg(arg2).arg(arg3).arg(arg4));
    }

    template<typename T1>
    static void info(const QString& format, const T1& arg1) {
        info(QString(format).arg(arg1));
    }
    template<typename T1, typename T2>
    static void info(const QString& format, const T1& arg1, const T2& arg2) {
        info(QString(format).arg(arg1).arg(arg2));
    }
    template<typename T1, typename T2, typename T3>
    static void info(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3) {
        info(QString(format).arg(arg1).arg(arg2).arg(arg3));
    }
    template<typename T1, typename T2, typename T3, typename T4>
    static void info(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        info(QString(format).arg(arg1).arg(arg2).arg(arg3).arg(arg4));
    }

    template<typename T1>
    static void warning(const QString& format, const T1& arg1) {
        warning(QString(format).arg(arg1));
    }
    template<typename T1, typename T2>
    static void warning(const QString& format, const T1& arg1, const T2& arg2) {
        warning(QString(format).arg(arg1).arg(arg2));
    }
    template<typename T1, typename T2, typename T3>
    static void warning(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3) {
        warning(QString(format).arg(arg1).arg(arg2).arg(arg3));
    }
    template<typename T1, typename T2, typename T3, typename T4>
    static void warning(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        warning(QString(format).arg(arg1).arg(arg2).arg(arg3).arg(arg4));
    }

    template<typename T1>
    static void error(const QString& format, const T1& arg1) {
        error(QString(format).arg(arg1));
    }
    template<typename T1, typename T2>
    static void error(const QString& format, const T1& arg1, const T2& arg2) {
        error(QString(format).arg(arg1).arg(arg2));
    }
    template<typename T1, typename T2, typename T3>
    static void error(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3) {
        error(QString(format).arg(arg1).arg(arg2).arg(arg3));
    }
    template<typename T1, typename T2, typename T3, typename T4>
    static void error(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        error(QString(format).arg(arg1).arg(arg2).arg(arg3).arg(arg4));
    }

    template<typename T1>
    static void critical(const QString& format, const T1& arg1) {
        critical(QString(format).arg(arg1));
    }
    template<typename T1, typename T2>
    static void critical(const QString& format, const T1& arg1, const T2& arg2) {
        critical(QString(format).arg(arg1).arg(arg2));
    }
    template<typename T1, typename T2, typename T3>
    static void critical(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3) {
        critical(QString(format).arg(arg1).arg(arg2).arg(arg3));
    }
    template<typename T1, typename T2, typename T3, typename T4>
    static void critical(const QString& format, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        critical(QString(format).arg(arg1).arg(arg2).arg(arg3).arg(arg4));
    }

    /**
     * @brief Get default log directory for this platform
     * @return Path to log directory (e.g., ~/.config/sound2osc/logs on Linux)
     */
    static QString getDefaultLogDir();

    /**
     * @brief Convert level enum to string
     */
    static QString levelToString(Level level);

private:
    Logger() = default;
    ~Logger() = default;

    static Logger& instance();
    void log(Level level, const QString& message);
    void writeToConsole(Level level, const QString& formattedMessage);
    void writeToFile(const QString& formattedMessage);
    void writeToSystem(Level level, const QString& message);
    QString formatMessage(Level level, const QString& message);

    // Platform-specific system logging
    void initSystemLogging(const QString& appName);
    void shutdownSystemLogging();

    QMutex m_mutex;
    Level m_level = Level::Info;
    int m_outputs = static_cast<int>(Output::Console);
    QString m_format = "[%timestamp%] [%level%] %message%";
    QString m_appName = "sound2osc";
    
    std::unique_ptr<QFile> m_logFile;
    std::unique_ptr<QTextStream> m_logStream;
    
    QVector<LogHandler> m_handlers;
    bool m_initialized = false;
};

// Convenience operators for Output flags
inline int operator|(Logger::Output a, Logger::Output b) {
    return static_cast<int>(a) | static_cast<int>(b);
}

inline int operator|(int a, Logger::Output b) {
    return a | static_cast<int>(b);
}

} // namespace sound2osc

#endif // SOUND2OSC_LOGGING_LOGGER_H
