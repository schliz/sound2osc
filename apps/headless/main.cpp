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
#include <sound2osc/config/PresetManager.h>
#include <sound2osc/audio/MonoAudioBuffer.h>
#include <sound2osc/audio/QAudioInputWrapper.h>
#include <sound2osc/dsp/FFTAnalyzer.h>
#include <sound2osc/osc/OSCNetworkManager.h>
#include <sound2osc/bpm/BPMDetector.h>
#include <sound2osc/bpm/BPMOscControler.h>
#include <sound2osc/trigger/TriggerGenerator.h>
#include <sound2osc/core/versionInfo.h>

#include <csignal>
#include <iostream>

using namespace sound2osc;

// Global flag for graceful shutdown
static bool g_running = true;

void signalHandler(int signum)
{
    Logger::info("Received signal %1, shutting down...", signum);
    g_running = false;
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

    // Create audio buffer and input wrapper (needed for device listing)
    MonoAudioBuffer buffer(NUM_SAMPLES * 4);
    QAudioInputWrapper audioInput(&buffer);

    // Handle --list-devices
    if (parser.isSet(listDevicesOption)) {
        QStringList devices = audioInput.getAvailableInputs();
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

    // Configuration setup
    QString configPath;
    if (parser.isSet(configOption)) {
        configPath = parser.value(configOption);
    } else {
        configPath = JsonConfigStore::getDefaultConfigPath("sound2osc-headless");
    }

    auto configStore = std::make_shared<JsonConfigStore>(configPath);
    configStore->load();

    SettingsManager settings(configStore);
    settings.load();

    // Apply command line overrides
    if (parser.isSet(oscHostOption)) {
        settings.setOscIpAddress(parser.value(oscHostOption));
    }
    if (parser.isSet(oscPortOption)) {
        bool ok;
        int port = parser.value(oscPortOption).toInt(&ok);
        if (ok && port > 0 && port < 65536) {
            settings.setOscUdpTxPort(static_cast<quint16>(port));
        }
    }

    // Set up audio input
    QString inputDevice = parser.isSet(inputDeviceOption) 
        ? parser.value(inputDeviceOption) 
        : settings.inputDeviceName();

    if (!inputDevice.isEmpty()) {
        Logger::info("Setting audio input: %1", inputDevice);
        audioInput.setInputByName(inputDevice);
    }

    // Show active input
    Logger::info("Active audio input: %1", audioInput.getActiveInputName());

    // Create OSC manager
    OSCNetworkManager osc;
    osc.setIpAddress(QHostAddress(settings.oscIpAddress()));
    osc.setUdpTxPort(settings.oscUdpTxPort());
    osc.setEnabled(true);

    Logger::info("OSC output: %1:%2", settings.oscIpAddress(), settings.oscUdpTxPort());

    // Create BPM detection components
    BPMOscControler bpmOsc(osc);
    BPMDetector bpmDetector(buffer, &bpmOsc);

    // Create trigger generators container
    QVector<TriggerGeneratorInterface*> triggerContainer;
    
    // Create trigger generators
    TriggerGenerator* bass = new TriggerGenerator("bass", &osc, true, false, 80);
    TriggerGenerator* loMid = new TriggerGenerator("loMid", &osc, true, false, 400);
    TriggerGenerator* hiMid = new TriggerGenerator("hiMid", &osc, true, false, 1000);
    TriggerGenerator* high = new TriggerGenerator("high", &osc, true, false, 5000);
    TriggerGenerator* envelope = new TriggerGenerator("envelope", &osc, false);
    TriggerGenerator* silence = new TriggerGenerator("silence", &osc, false, true);

    triggerContainer.append(bass);
    triggerContainer.append(loMid);
    triggerContainer.append(hiMid);
    triggerContainer.append(high);
    triggerContainer.append(envelope);
    triggerContainer.append(silence);

    // Create FFT analyzer
    FFTAnalyzer fft(buffer, triggerContainer);

    // Audio input is started automatically by QAudioInputWrapper constructor

    // FFT update timer (44 Hz)
    QTimer fftTimer;
    QObject::connect(&fftTimer, &QTimer::timeout, [&]() {
        if (g_running) {
            fft.calculateFFT(false);
        }
    });
    fftTimer.start(1000 / 44);  // ~44 Hz

    // BPM update timer
    QTimer bpmTimer;
    QObject::connect(&bpmTimer, &QTimer::timeout, [&]() {
        if (g_running) {
            bpmDetector.detectBPM();
        }
    });
    bpmTimer.start(1000 / 44);

    // Status update timer (every 5 seconds)
    QTimer statusTimer;
    QObject::connect(&statusTimer, &QTimer::timeout, [&]() {
        if (g_running) {
            float bpm = bpmDetector.getBPM();
            Logger::debug("Status: BPM=%1, Audio=%2", 
                         QString::number(static_cast<double>(bpm), 'f', 1),
                         audioInput.getActiveInputName());
        }
    });
    statusTimer.start(5000);

    Logger::info("Headless mode running. Press Ctrl+C to stop.");

    // Run event loop
    int result = app.exec();

    // Cleanup
    Logger::info("Shutting down...");
    
    fftTimer.stop();
    bpmTimer.stop();
    statusTimer.stop();

    // Delete trigger generators
    delete bass;
    delete loMid;
    delete hiMid;
    delete high;
    delete envelope;
    delete silence;

    // Save settings
    settings.save();

    Logger::info("Goodbye!");
    Logger::shutdown();

    return result;
}
