// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
//
// sound2osc Headless Application
// 
// This is a minimal headless (no GUI) implementation demonstrating
// that the core library can operate independently. It provides a
// foundation for future web-ui or daemon deployments.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QDir>

#include <sound2osc/logging/Logger.h>
#include <sound2osc/config/JsonConfigStore.h>
#include <sound2osc/config/SettingsManager.h>
#include <sound2osc/core/Sound2OscEngine.h>
#include <sound2osc/core/versionInfo.h>

#include <csignal>
#include <iostream>

using namespace sound2osc;

// Global engine pointer for signal handling
static Sound2OscEngine* g_engine = nullptr;

void signalHandler(int signum)
{
    Logger::info("Received signal %1, shutting down...", signum);
    if (g_engine) {
        g_engine->stop();
    }
    QCoreApplication::quit();
}

void printBanner()
{
    std::cout << "\n"
              << "  sound2osc headless v" << VERSION_STRING.toStdString() << "\n"
              << "  Audio analysis to OSC bridge\n"
              << "  (C) Electronic Theatre Controls, Inc.\n"
              << "  (C) Christian Schliz <code+sound2osc@foxat.de>\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("sound2osc-headless");
    app.setApplicationVersion(VERSION_STRING);
    app.setOrganizationName("sound2osc");
    app.setOrganizationDomain("sound2osc");

    // Setup signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Command line parsing
    QCommandLineParser parser;
    parser.setApplicationDescription("sound2osc Headless - Audio analysis to OSC without GUI");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configOption(
        QStringList() << "c" << "config",
        "Path to JSON configuration file",
        "file"
    );
    parser.addOption(configOption);

    QCommandLineOption oscHostOption(
        QStringList() << "H" << "host",
        "OSC destination host (default: 127.0.0.1)",
        "address",
        "127.0.0.1"
    );
    parser.addOption(oscHostOption);

    QCommandLineOption oscPortOption(
        QStringList() << "p" << "port",
        "OSC destination port (default: 9000)",
        "port",
        "9000"
    );
    parser.addOption(oscPortOption);

    QCommandLineOption inputDeviceOption(
        QStringList() << "i" << "input",
        "Audio input device name (use 'list' to show available)",
        "device"
    );
    parser.addOption(inputDeviceOption);

    QCommandLineOption verboseOption(
        "verbose",
        "Enable verbose output (debug logging)"
    );
    parser.addOption(verboseOption);

    QCommandLineOption listDevicesOption(
        "list-devices",
        "List available audio input devices and exit"
    );
    parser.addOption(listDevicesOption);

    parser.process(app);

    // Initialize logging
    Logger::initialize("sound2osc-headless", static_cast<int>(Logger::Output::Console));
    
    if (parser.isSet(verboseOption)) {
        Logger::setLogLevel(Logger::Level::Debug);
    } else {
        Logger::setLogLevel(Logger::Level::Info);
    }

    printBanner();

    // Configuration setup
    QString configPath;
    if (parser.isSet(configOption)) {
        configPath = parser.value(configOption);
    } else {
        configPath = JsonConfigStore::getDefaultConfigPath("sound2osc-headless");
    }

    auto configStore = std::make_shared<JsonConfigStore>(configPath);
    configStore->load();

    auto settings = std::make_shared<SettingsManager>(configStore);
    settings->load();

    // Create the Engine
    Sound2OscEngine engine(settings);
    g_engine = &engine;

    // Handle --list-devices
    // We need to initialize the engine components to list devices, but not start processing
    if (parser.isSet(listDevicesOption)) {
        QStringList devices = engine.audioInput()->getAvailableInputs();
        std::cout << "Available audio input devices:" << std::endl;
        for (int i = 0; i < devices.size(); ++i) {
            std::cout << "  [" << i << "] " << devices[i].toStdString() << std::endl;
        }
        if (devices.isEmpty()) {
            std::cout << "  (no devices found)" << std::endl;
        }
        return 0;
    }

    Logger::info("Starting sound2osc Headless v%1", QString(VERSION_STRING));

    // Apply command line overrides to settings
    if (parser.isSet(oscHostOption)) {
        settings->setOscIpAddress(parser.value(oscHostOption));
    }
    if (parser.isSet(oscPortOption)) {
        bool ok;
        int port = parser.value(oscPortOption).toInt(&ok);
        if (ok && port > 0 && port < 65536) {
            settings->setOscUdpTxPort(static_cast<quint16>(port));
        }
    }
    
    if (parser.isSet(inputDeviceOption)) {
        settings->setInputDeviceName(parser.value(inputDeviceOption));
    }

    // Start the engine
    engine.start();

    Logger::info("Active audio input: %1", engine.audioInput()->getActiveInputName());
    Logger::info("OSC output: %1:%2", settings->oscIpAddress(), settings->oscUdpTxPort());
    Logger::info("Headless mode running. Press Ctrl+C to stop.");

    // Run event loop
    int result = app.exec();

    // Cleanup
    Logger::info("Shutting down...");
    engine.stop();
    
    // Save settings
    settings->save();

    Logger::info("Goodbye!");
    Logger::shutdown();

    return result;
}
