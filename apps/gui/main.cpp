// SPDX-License-Identifier: MIT
// Copyright (c) 2016 Electronic Theatre Controls, Inc.
// Copyright (c) 2026-present Christian Schliz <code+sound2osc@foxat.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "controllers/MainController.h"
#include "utils/SettingsMigration.h"

#include <sound2osc/core/AppInfo.h>
#include <sound2osc/config/SettingsManager.h>
#include <sound2osc/config/PresetManager.h>
#include <sound2osc/logging/Logger.h>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QSettings>
#include <QScreen>
#include <QSplashScreen>
#include <QStandardPaths>


int main(int argc, char *argv[]) {
    using namespace sound2osc;
    
    // Qt6: High DPI scaling is enabled by default, AA_EnableHighDpiScaling is deprecated
    QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/icons/appicon.ico"));

	// Configure application identity using AppInfo
	// This allows easy rebranding for forks while maintaining backward compatibility
	QCoreApplication::setOrganizationName(AppInfo::organizationName());
	QCoreApplication::setApplicationName(AppInfo::applicationName());
	QCoreApplication::setApplicationVersion(AppInfo::applicationVersion());
	QSettings::setDefaultFormat(QSettings::IniFormat);

	// Initialize logging
	Logger::info(QString("Starting %1 v%2")
	    .arg(AppInfo::applicationDisplayName())
	    .arg(AppInfo::applicationVersion()));
    Logger::info("(C) Electronic Theatre Controls, Inc.");
    Logger::info("(C) Christian Schliz <code+sound2osc@foxat.de>");

	// ----------- Settings Migration --------
	// Check for legacy QSettings and migrate to new JSON format if needed
	QString presetDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	auto settingsManager = std::make_shared<SettingsManager>();
	auto presetManager = std::make_unique<PresetManager>(presetDir);
	
	if (SettingsMigration::hasLegacySettings()) {
	    Logger::info("Legacy settings detected, starting migration...");
	    SettingsMigration::migrate(settingsManager.get(), presetManager.get());
	}
	
	// Load settings (either migrated or existing)
	settingsManager->load();

	// ----------- Show Splash Screen --------
    // Splash screen disabled to remove ETC branding (user can re-enable with custom logo)
	// QPixmap pixmap(":/images/icons/logo.png");
	// QSplashScreen splash(pixmap);
	// splash.show();

	// call processEvents() to load image in Splash Screen:
	// app.processEvents();

	// ----------- Load QML ---------------

	// create QmlEngine and MainController:
	QQmlApplicationEngine engine;
    MainController* controller = new MainController(&engine, settingsManager, presetManager.get());

	// set global QML variable "controller" to a pointer to the MainController:
    engine.rootContext()->setContextProperty("controller", controller);
    
    // Expose AppInfo to QML for branding
    engine.rootContext()->setContextProperty("appVersion", AppInfo::applicationVersion());
    engine.rootContext()->setContextProperty("appName", AppInfo::applicationDisplayName());

	// quit QGuiApplication when quit signal is emitted:
	QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);
	// call onExit() method of controller when app is about to quit:
    QObject::connect(&app, &QCoreApplication::aboutToQuit, controller, &MainController::onExit);

    controller->initBeforeQmlIsLoaded();
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    controller->initAfterQmlIsLoaded();

	// ---------- Hide Splash Screen ---------
	// splash.hide();
	
	Logger::info("Application initialized successfully");

	// start main application loop:
	return app.exec();
}
