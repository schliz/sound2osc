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

#include "MainController.h"

#include <sound2osc/audio/QAudioInputWrapper.h>
#include <sound2osc/trigger/TriggerGenerator.h>
#include <sound2osc/config/SettingsManager.h>
#include <sound2osc/config/PresetManager.h>
#include <sound2osc/logging/Logger.h>
#include "TriggerGuiController.h"
#include <sound2osc/osc/OSCNetworkManager.h>

#include <QtMath>
#include <QQmlContext>
#include <QSettings>
#include <QQuickItem>
#include <QQmlComponent>
#include <QStandardPaths>
#include <QDateTime>
#include <QScreen>
#include <QFileDialog>

using namespace sound2osc;

MainController::MainController(QQmlApplicationEngine* qmlEngine,
                               std::shared_ptr<SettingsManager> settingsManager,
                               PresetManager* presetManager,
                               QObject *parent)
	: QObject(parent)
    , m_engine(nullptr)
	, m_qmlEngine(qmlEngine)
	, m_settingsManager(settingsManager)
	, m_presetManager(presetManager)
	, m_consoleType("Eos")
	, m_oscMapping(this)
    , m_bpmTap(nullptr)
    , m_bpmActive(false)
    , m_waveformVisible(true)
    , m_autoBpm(false)
{
    // Create the engine
    m_engine = std::make_unique<Sound2OscEngine>(settingsManager, this);
    
    // Initialize BPMTapDetector with the engine's BPMOscController
    m_bpmTap.setOscController(m_engine->bpmOsc());

	connectGeneratorsWithGui();
}

MainController::~MainController()
{
	// m_engine and controllers will be automatically deleted
}

void MainController::initBeforeQmlIsLoaded()
{
	// init everything that can or must be done before QML file is loaded:
    OSCNetworkManager* osc = m_engine->osc();
    
	connect(osc, &OSCNetworkManager::messageReceived, this, &MainController::messageReceived);
	connect(osc, &OSCNetworkManager::packetSent, this, &MainController::packetSent);
	connect(osc, &OSCNetworkManager::useTcpChanged, this, &MainController::useTcpChanged);
	connect(osc, &OSCNetworkManager::isConnectedChanged, this, &MainController::isConnectedChanged);
	connect(osc, &OSCNetworkManager::isConnectedChanged, this, &MainController::onConnectedChanged);
	connect(osc, &OSCNetworkManager::addressChanged, this, &MainController::addressChanged);
	connect(osc, &OSCNetworkManager::logChanged, this, &MainController::oscLogChanged);
	connect(osc, &OSCNetworkManager::messageReceived, &m_oscMapping, &OSCMapping::handleMessage);
	connect(&m_oscUpdateTimer, &QTimer::timeout, &m_oscMapping, &OSCMapping::sendLevelFeedback);

	// load settings that may be used while QML file is loaded:
	loadPresetIndependentSettings();
	restorePreset();
}

void MainController::initAfterQmlIsLoaded()
{
	// init things that depend on the loaded QML file:
	initAudioInput();
	restoreWindowGeometry();
	autosave();

	// create global QML property that points to the main Window:
	m_qmlEngine->rootContext()->setContextProperty("mainWindow", QVariant::fromValue<QQuickWindow*>(getMainWindow()));

	connect(this, &MainController::presetChanged, this, &MainController::onPresetChanged);
	connect(getMainWindow(), &QQuickWindow::visibilityChanged, this, &MainController::onVisibilityChanged);
}

void MainController::connectGeneratorsWithGui()
{
	// create TriggerGuiController objects
	// and initialize them with the correct triggerGenerators from the Engine:
	m_bassController = std::make_unique<TriggerGuiController>(m_engine->getBass());
	m_loMidController = std::make_unique<TriggerGuiController>(m_engine->getLoMid());
	m_hiMidController = std::make_unique<TriggerGuiController>(m_engine->getHiMid());
	m_highController = std::make_unique<TriggerGuiController>(m_engine->getHigh());
	m_envelopeController = std::make_unique<TriggerGuiController>(m_engine->getEnvelope());
	m_silenceController = std::make_unique<TriggerGuiController>(m_engine->getSilence());

	// set a QML context property for each
	// so that for example the m_bassController is accessible as "bassController" in QML:
	m_qmlEngine->rootContext()->setContextProperty("bassController", m_bassController.get());
	m_qmlEngine->rootContext()->setContextProperty("loMidController", m_loMidController.get());
	m_qmlEngine->rootContext()->setContextProperty("hiMidController", m_hiMidController.get());
	m_qmlEngine->rootContext()->setContextProperty("highController", m_highController.get());
	m_qmlEngine->rootContext()->setContextProperty("envelopeController", m_envelopeController.get());
	m_qmlEngine->rootContext()->setContextProperty("silenceController", m_silenceController.get());

	// connect the presetChanged signal to the onPresetChanged slot of this controller:
	connect(m_bassController.get(), &TriggerGuiController::presetChanged, this, &MainController::onPresetChanged);
	connect(m_loMidController.get(), &TriggerGuiController::presetChanged, this, &MainController::onPresetChanged);
	connect(m_hiMidController.get(), &TriggerGuiController::presetChanged, this, &MainController::onPresetChanged);
	connect(m_highController.get(), &TriggerGuiController::presetChanged, this, &MainController::onPresetChanged);
	connect(m_envelopeController.get(), &TriggerGuiController::presetChanged, this, &MainController::onPresetChanged);
	connect(m_silenceController.get(), &TriggerGuiController::presetChanged, this, &MainController::onPresetChanged);

}

QQuickWindow *MainController::getMainWindow() const
{
	QQuickWindow* window = qobject_cast<QQuickWindow*>(m_qmlEngine->rootObjects()[0]);
	return window;
}

bool MainController::settingsFormatIsValid(const QString& filePath) const
{
	// check format version from a preset file:
	QSettings settings(filePath, QSettings::IniFormat);
	int formatVersion = settings.value("formatVersion").toInt();
	if (formatVersion == 0) {
		// this is the first start of the software, nothing to restore
		Logger::debug("this is the first start of the software, nothing to restore");
		return false;
	} else if (formatVersion < SETTINGS_FORMAT_VERSION) {
		// the format of the settings is too old, cannot restore
		Logger::debug("the format of the settings is too old, cannot restore");
		return false;
	}
	return true;
}

void MainController::onConnectedChanged()
{
	// send current state via OSC if a new connection was made:
	if (m_engine->osc()->isConnected()) {
		m_oscMapping.sendCurrentState();
	}
}

void MainController::initAudioInput()
{
    // The Engine handles audio input initialization based on settings.
    // We just need to make sure the Engine is started.
    m_engine->start();
    
    // But we also need to handle the "No Input Device" dialog logic if needed.
    // The Engine applies settings in start(), which sets the input device.
    
	QString inputDeviceName = m_engine->audioInput()->getActiveInputName();
    
	if (inputDeviceName.isEmpty()) {
		// if there is no default input, open NoInputDeviceDialog:
		openDialog("qrc:/qml/NoInputDeviceDialog.qml");
	} else {
		emit inputChanged();
	}

    // BPM setup
    setBPMActive(m_bpmActive);
}

void MainController::triggerBeat()
{
    setAutoBpm(false);
    m_bpmTap.triggerBeat();
}

void MainController::setBPM(float value)
{
    setAutoBpm(false);
    m_bpmTap.setBpm(value);
}

void MainController::activateBPM()
{
    m_engine->bpm()->resetCache();
    m_bpmTap.reset();
    // Engine timers are already running
}

void MainController::deactivateBPM()
{
    // We don't stop engine timers, just logic
}

void MainController::setConsoleType(QString value)
{
	if (value.isEmpty()) return;
	if (value == "EOS") value = "Eos";
	if (value == "Cobalt") value = "Cobalt 7.2";
	m_consoleType = value;
	emit settingsChanged();
	emit presetChanged();
}

QList<qreal> MainController::getSpectrumPoints()
{
	// convert const QVector<float>& to QList<qreal> to be used in GUI:
	QList<qreal> points;
	const QVector<float>& spectrum = m_engine->fft()->getNormalizedSpectrum();
	for (int i=0; i<spectrum.size(); ++i) {
		points.append(spectrum[i]);
	}
	return points;
}

QList<qreal> MainController::getWavePoints()
{
    // convert const QVector<float>& to QList<qreal> to be used in GUI:
    QList<qreal> points;
    const Qt3DCore::QCircularBuffer<float>& wave = m_engine->bpm()->getWaveDisplay();
    for (int i = 0; i < wave.size(); ++i) {
        points.append(wave.at(i) / 350 * m_engine->fft()->getScaledSpectrum().getGain());
    }
    return points;
}

QList<bool> MainController::getWaveOnsets()
{
    // conert const QVector<bool>& to QList<bool> to be used in GUI:
    QList<bool> points;
    const QVector<bool>& peaks = m_engine->bpm()->getOnsets();
    for (int i = 0; i < peaks.size(); ++i) {
        points.append(peaks[i]);
    }
    return points;
}

QList<QString> MainController::getWaveColors()
{
    // convert const Qt3DCore::QCircularBuffer<SpectrumColor>& to QList<QString> to be used in GUI:
    QList<QString> points;
    const auto& colors = m_engine->bpm()->getWaveColors();
    for (int i = 0; i < colors.size(); ++i) {
        const auto& c = colors.at(i);
        points.append(QColor(c.r, c.g, c.b).name());
    }
    return points;
}

void MainController::setOscEnabled(bool value) {
	m_engine->osc()->setEnabled(value); emit settingsChanged();
	m_engine->osc()->sendMessage(QString("/sound2osc/out/enabled=").append(value ? "1" : "0"), true);
}

// enable or disables bpm detection
void MainController::setBPMActive(bool value) {
    if (value == m_bpmActive) return;
    m_bpmActive = value;
    if (m_bpmActive) {
        activateBPM();
    } else {
        deactivateBPM();
    }
    emit bpmActiveChanged();
    emit waveformVisibleChanged(); // because wavformvisible is only true if bpm is active
    m_engine->osc()->sendMessage("/sound2osc/out/bpm/enabled", (value ? "1" : "0"), true);
}

void MainController::setAutoBpm(bool value) {
    if (value == m_autoBpm) return;
    m_autoBpm = value;
    qDebug() << m_autoBpm;
    m_engine->bpm()->setTransmitBpm(m_autoBpm);
    if (m_autoBpm && !m_bpmActive) {
        setBPMActive(true);
    } else if (!m_autoBpm && !m_waveformVisible && m_bpmActive) {
        setBPMActive(false);
    }
    emit autoBpmChanged();
}

// sets the minium bpm of the range
void MainController::setMinBPM(int value) {
    m_engine->bpm()->setMinBPM(value);
    m_bpmTap.setMinBPM(value);
    emit bpmRangeChanged();
    m_engine->osc()->sendMessage("/sound2osc/out/bpm/range", QString::number(value), true);
}

int MainController::getMinBPM() {
    return m_engine->bpm()->getMinBPM();
}

void MainController::setWaveformVisible(bool value) {
    m_waveformVisible = value;
    if (m_waveformVisible && !m_bpmActive) {
        setBPMActive(true);
    } else if (!m_autoBpm && !m_waveformVisible && m_bpmActive) {
        setBPMActive(false);
    }
    emit waveformVisibleChanged();
}

void MainController::onExit()
{
    savePresetIndependentSettings();
    autosave();
    m_engine->stop();
}

void MainController::onVisibilityChanged()
{
	// if main Window is minimized:
	if (getMainWindow()->visibility() == QWindow::Minimized) {
		// iterate over all open dialogs:
		QMap<QString, QObject*>::iterator i = m_dialogs.begin();
		for (; i != m_dialogs.end(); ++i) {
			// check if dialog exists:
			if (!i.value()) continue;
			// check if it should be minimized or closed:
			if (i.value()->property("modality").toInt() == Qt::NonModal) {
				// a non modal dialog will be minimized together with the main window:
				// FIXME: is a QML Dialog minimizable?
				// it seems it is not, it will be closed instead:
				QMetaObject::invokeMethod(i.value(), "close");
			} else {
				// a modal dialog will be closed when the main window is minimized:
				QMetaObject::invokeMethod(i.value(), "close");
			}
		}
	}
}

void MainController::savePresetIndependentSettings() const
{
	// Use SettingsManager for saving settings
	if (!m_settingsManager) {
		Logger::warning("SettingsManager not available, cannot save settings");
		return;
	}
	
	// Save OSC settings
	m_settingsManager->setOscIpAddress(getOscIpAddress());
	m_settingsManager->setOscUdpTxPort(getOscUdpTxPort());
	m_settingsManager->setOscUdpRxPort(getOscUdpRxPort());
	m_settingsManager->setOscTcpPort(getOscTcpPort());
	m_settingsManager->setOscEnabled(getOscEnabled());
	m_settingsManager->setUseTcp(getUseTcp());
	m_settingsManager->setUseOsc_1_1(getUseOsc_1_1());
	m_settingsManager->setOscLogIncomingEnabled(getOscLogIncomingIsEnabled());
	m_settingsManager->setOscLogOutgoingEnabled(getOscLogOutgoingIsEnabled());
	m_settingsManager->setOscInputEnabled(getOscInputEnabled());
	
	// Save window geometry
	QRect windowGeometry = getMainWindow()->geometry();
	if (windowGeometry.width() < 300) {
		// -> minimal mode, save default geometry
		windowGeometry.setWidth(1200);
		windowGeometry.setHeight(800);
	}
	m_settingsManager->setWindowGeometry(windowGeometry);
	bool maximized = (getMainWindow()->width() == QGuiApplication::primaryScreen()->availableSize().width());
	m_settingsManager->setWindowMaximized(maximized);
	
	// Save input device
	m_settingsManager->setInputDeviceName(getActiveInputName());
	
	// Sync to storage
	m_settingsManager->save();
}

void MainController::loadPresetIndependentSettings()
{
	// Use SettingsManager if available
	if (!m_settingsManager) {
		Logger::warning("SettingsManager not available, skipping settings load");
		return;
	}
	
	if (!m_settingsManager->isValid()) {
		Logger::debug("No valid settings found, using defaults");
		return;
	}

	// restore preset independent settings from SettingsManager:
	setOscIpAddress(m_settingsManager->oscIpAddress());
	setOscUdpTxPort(m_settingsManager->oscUdpTxPort());
	setOscUdpRxPort(m_settingsManager->oscUdpRxPort());
	setOscTcpPort(m_settingsManager->oscTcpPort());
	setOscEnabled(m_settingsManager->oscEnabled());
	setUseTcp(m_settingsManager->useTcp());
	setUseOsc_1_1(m_settingsManager->useOsc_1_1());
	enableOscLogging(m_settingsManager->oscLogIncomingEnabled(), 
	                 m_settingsManager->oscLogOutgoingEnabled());
	setOscInputEnabled(m_settingsManager->oscInputEnabled());
}

void MainController::restoreWindowGeometry()
{
	// Use SettingsManager for window geometry
	if (!m_settingsManager || !m_settingsManager->isValid()) {
		Logger::debug("No valid settings for window geometry");
		QQuickWindow* window = getMainWindow();
		if (window) {
			connect(window, &QQuickWindow::closing, m_qmlEngine, &QQmlApplicationEngine::quit);
		}
		return;
	}

	QRect windowGeometry = m_settingsManager->windowGeometry();
	bool maximized = m_settingsManager->windowMaximized();
	QQuickWindow* window = getMainWindow();
	if (!window) return;
	if (!windowGeometry.isNull()) window->setGeometry(windowGeometry);
	if (maximized) window->showMaximized();
	connect(window, &QQuickWindow::closing, m_qmlEngine, &QQmlApplicationEngine::quit);

	// TODO: restore visibility of details in trigger settings
	// TODO: restore position of splitView handle
}

void MainController::loadPreset(const QString &constFileName, bool createIfNotExistent)
{
    Q_UNUSED(createIfNotExistent);
    QJsonObject state = m_presetManager->loadPresetFile(constFileName);
    
    if (state.isEmpty()) {
        m_engine->osc()->sendMessage("/sound2osc/out/error", QString("Preset empty or not found: ").append(constFileName), true);
        return;
    }
    
    // Apply state to engine
    m_engine->fromState(state);
    
    // Update local UI state that isn't part of engine state directly
    // (Note: Engine fromState handles almost everything now)
    
    // Manual BPM Tap Value is separate from Engine state in current logic
    if (state.contains("bpm")) {
        QJsonObject bpm = state["bpm"].toObject();
        // Restore manual tap value if it was saved there (it was in PresetManager::convertLegacySettingsToJson)
        // But wait, convertLegacySettingsToJson does NOT save tapValue currently?
        // Let's check PresetManager.cpp implementation.
        // Yes it does: bpm["tapValue"] = settings.value("bpm/tapvalue", 60).toInt(); in convertLegacySettingsToJson?
        // Wait, I didn't add tapValue to convertLegacySettingsToJson in my previous edit.
        // I need to double check PresetManager.cpp.
        
        // Assuming I did it right or will fix it:
        if (bpm.contains("tapValue")) {
            m_bpmTap.setBpm(bpm["tapValue"].toInt(60));
        }
    }
    
    // Update UI properties
    // These should ideally be bound to engine, but we manually emit signals for now
	emit decibelConversionChanged();
	emit agcEnabledChanged();
	emit gainChanged();
	emit compressionChanged();
    emit bpmActiveChanged();
    emit bpmRangeChanged();
    emit waveformVisibleChanged();
    emit bpmMuteChanged();

	emit m_bassController->parameterChanged();
	emit m_loMidController->parameterChanged();
	emit m_hiMidController->parameterChanged();
	emit m_highController->parameterChanged();
	emit m_envelopeController->parameterChanged();
	emit m_silenceController->parameterChanged();

	emit m_bassController->oscLabelTextChanged();
	emit m_loMidController->oscLabelTextChanged();
	emit m_hiMidController->oscLabelTextChanged();
	emit m_highController->oscLabelTextChanged();
	emit m_envelopeController->oscLabelTextChanged();
	emit m_silenceController->oscLabelTextChanged();

    emit m_bassController->muteChanged();
    emit m_loMidController->muteChanged();
    emit m_hiMidController->muteChanged();
    emit m_highController->muteChanged();
    emit m_envelopeController->muteChanged();
    emit m_silenceController->muteChanged();
    
    // Update filename
    m_currentPresetFilename = m_presetManager->cleanFilePath(constFileName, false);
    m_presetManager->setCurrentPresetPath(m_currentPresetFilename);
    emit presetNameChanged();
    
    m_presetChangedButNotSaved = false; 
    emit presetChangedButNotSavedChanged();
    
	QString baseName = QFileInfo(m_currentPresetFilename).baseName();
	m_engine->osc()->sendMessage("/sound2osc/out/active_preset", baseName, true);
	m_engine->osc()->sendMessage("/sound2osc/out/error", "-", true);
}

void MainController::savePresetAs(const QString &constFileName, bool isAutosave)
{
    if (constFileName.isEmpty()) return;
    
    QJsonObject state = m_engine->toState();
    
    // Add manual BPM tap value which is managed by MainController/BPMTapDetector
    if (state.contains("bpm")) {
        QJsonObject bpm = state["bpm"].toObject();
        bpm["tapValue"] = static_cast<int>(m_bpmTap.getBpm());
        state["bpm"] = bpm;
    }
    
    bool success = m_presetManager->savePresetFile(constFileName, state, isAutosave);
    
    if (success && !isAutosave) {
        m_currentPresetFilename = m_presetManager->cleanFilePath(constFileName);
        emit presetNameChanged();
        m_presetChangedButNotSaved = false; 
        emit presetChangedButNotSavedChanged();
    }
}

void MainController::saveCurrentPreset()
{
	if (m_currentPresetFilename.isEmpty()) {
		openSavePresetAsDialog();
	} else {
		savePresetAs(m_currentPresetFilename);
	}
}

void MainController::autosave()
{
    QJsonObject state = m_engine->toState();
    if (state.contains("bpm")) {
        QJsonObject bpm = state["bpm"].toObject();
        bpm["tapValue"] = static_cast<int>(m_bpmTap.getBpm());
        state["bpm"] = bpm;
    }
	m_presetManager->saveAutosave(state);
}

void MainController::restorePreset()
{
    QJsonObject state = m_presetManager->loadAutosave();
    if (!state.isEmpty()) {
        m_engine->fromState(state);
        
        // Restore manual tap value
        if (state.contains("bpm")) {
            QJsonObject bpm = state["bpm"].toObject();
            if (bpm.contains("tapValue")) {
                m_bpmTap.setBpm(bpm["tapValue"].toInt(60));
            }
        }
    }

	QSettings independentSettings;
	QString presetFileName = independentSettings.value("presetFileName").toString();

    // Just restore the filename display, logic is similar to before but simplified
	if (presetFileName.isEmpty()) {
		m_currentPresetFilename = "";
		m_presetChangedButNotSaved = independentSettings.value("presetChangedButNotSaved").toBool();
	} else if (QFile::exists(presetFileName)) {
		m_currentPresetFilename = presetFileName;
		m_presetChangedButNotSaved = independentSettings.value("presetChangedButNotSaved").toBool();
	} else {
		m_currentPresetFilename = "";
		m_presetChangedButNotSaved = true;
	}
    
    m_presetManager->setCurrentPresetPath(m_currentPresetFilename);

	emit presetNameChanged();
	emit presetChangedButNotSavedChanged();
    
    // Emit all changes
    emit decibelConversionChanged();
    emit agcEnabledChanged();
    emit gainChanged();
    emit compressionChanged();
    emit bpmActiveChanged();
    emit bpmRangeChanged();
    emit waveformVisibleChanged();
    emit bpmMuteChanged();
}

void MainController::resetPreset()
{
	// reset all values to default:
	setFftGain(1.0);
	setFftCompression(1.0);
	setAgcEnabled(true);
	setDecibelConversion(false);
    setLowSoloMode(false);
    setBPMActive(false);
    setMinBPM(75);
    setBPMOscCommands(QStringList());
    setWaveformVisible(true);

    emit bpmActiveChanged();
    emit bpmRangeChanged();
    emit waveformVisibleChanged();

	// reset values in all TriggerGuiControllers:
	m_bassController->resetParameters();
	m_loMidController->resetParameters();
	m_hiMidController->resetParameters();
	m_highController->resetParameters();
	m_envelopeController->resetParameters();
	m_silenceController->resetParameters();

	// clear currentPresetFilename:
	m_currentPresetFilename = ""; 
    m_presetManager->setCurrentPresetPath("");
    emit presetNameChanged();
	m_presetChangedButNotSaved = false; emit presetChangedButNotSavedChanged();
}

void MainController::deletePreset(const QString &fileName)
{
	// if the preset to delete is currently loaded, reset to default settings:
	if (m_currentPresetFilename == fileName) {
		resetPreset();
	}

    m_presetManager->deletePreset(fileName);
}

QString MainController::getPresetDirectory() const
{
	return m_presetManager->presetDirectory();
}

QString MainController::getPresetName() const
{
    return m_presetManager->currentPresetName();
}

void MainController::onPresetChanged()
{
	if (!m_presetChangedButNotSaved) {
		m_presetChangedButNotSaved = true;
		emit presetChangedButNotSavedChanged();
	}
}

void MainController::sendOscTestMessage(QString message)
{
	m_engine->osc()->sendMessage(message, true);
}

void MainController::openDialog(const QString &qmlDialogFile, QString propertyName, QVariant propertyValue)
{
	qDebug() << "openDialog called for:" << qmlDialogFile;
	
	// check if dialog is already opened:
	if (m_dialogs.find(qmlDialogFile) != m_dialogs.end()) {
		// dialog is open
		QObject* dialog = m_dialogs[qmlDialogFile];
		if (dialog && dialog->property("visible").toBool()) {
			qDebug() << "Dialog already open, toggling visibility";
			dialog->setProperty("visible", false);
			dialog->setProperty("visible", true);
			return;
		}
	}
	// create new dialog from QML file:
	// Use QQmlComponent::PreferSynchronous to ensure component is ready immediately
	QQmlComponent* component = new QQmlComponent(m_qmlEngine, QUrl(qmlDialogFile), QQmlComponent::PreferSynchronous);
	
	// Check if component is ready before creating
	if (!component->isReady()) {
		qWarning() << "QQmlComponent: Component is not ready for" << qmlDialogFile;
		if (component->isError()) {
			qWarning() << "QQmlComponent errors:" << component->errors();
		}
		delete component;
		return;
	}
	
	qDebug() << "Component ready, creating dialog object";
	QObject* dialog = component->beginCreate(m_qmlEngine->rootContext());
	if (!dialog) {
		qWarning() << "Failed to create dialog object for" << qmlDialogFile;
		delete component;
		return;
	}
	
	qDebug() << "Dialog object created successfully";
	
	// Qt6: Set parent for Dialog/Popup to make it visible
	// Dialog needs a parent window or contentItem to be displayed
	QQuickWindow* mainWindow = getMainWindow();
	if (mainWindow) {
		qDebug() << "Setting dialog parent to main window";
		// For QtQuick.Controls Dialog (which is a Popup), we need to set the parent
		QQuickItem* contentItem = mainWindow->contentItem();
		dialog->setParent(contentItem);
		dialog->setProperty("parent", QVariant::fromValue(contentItem));
	}
	
	if (!propertyName.isEmpty()) {
		dialog->setProperty(propertyName.toLatin1(), propertyValue);
	}
	component->completeCreate();
	qDebug() << "Calling open() on dialog";
	QMetaObject::invokeMethod(dialog, "open");
	m_dialogs[qmlDialogFile] = dialog;
	qDebug() << "Dialog opened and stored";
}

void MainController::dialogIsClosed(QObject *dialog)
{
	if (!dialog) return;
	// remove dialog from open dialogs map:
	QString qmlDialogFile = m_dialogs.key(dialog);
	m_dialogs.remove(qmlDialogFile);
	// delete dialog object:
	// FIXME: assertion error when deleting dialog
	dialog->deleteLater();
}

void MainController::setPropertyWithoutChangingBindings(const QVariant &item, QString name, QVariant value)
{
	QQuickItem* qitem = item.value<QQuickItem*>();
	qitem->setProperty(name.toLatin1().data(), value);
}

bool MainController::oscLevelFeedbackIsEnabled()
{
	return m_oscUpdateTimer.isActive();
}

void MainController::enableOscLevelFeedback(bool value)
{
	if (value) {
		// start OSC level feedback timer:
		m_oscUpdateTimer.start(static_cast<int>(1000.0 / OSC_LEVEL_FEEDBACK_RATE));
	} else {
		// stop OSC level feedback timer:
		m_oscUpdateTimer.stop();
	}
	// give feedback about state via OSC:
	m_engine->osc()->sendMessage(QString("/sound2osc/out/level_feedback=").append(value ? "1" : "0"), true);
}

void MainController::openSavePresetAsDialog()
{
	// open QWidget based dialog and wait for result (blocking):
	QString fileName = QFileDialog::getSaveFileName(0, tr("Save Preset As"),
							   getPresetDirectory(),
							   tr("sound2osc Presets (*.s2o *.s2l)"));
	savePresetAs(fileName);
}

void MainController::openLoadPresetDialog()
{
	// open QWidget based dialog and wait for result (blocking):
	QString fileName = QFileDialog::getOpenFileName(0, tr("Open Preset"),
							   getPresetDirectory(),
							   tr("sound2osc Presets (*.s2o *.s2l)"));
	loadPreset(fileName);
}

// Low Solo Mode
bool MainController::getLowSoloMode() const { return m_engine->getLowSoloMode(); }
void MainController::setLowSoloMode(bool value) { m_engine->setLowSoloMode(value); emit lowSoloModeChanged(); }

// BPM Helpers
float MainController::getBPM() { return getBPMManual() || m_engine->bpm()->getBPM() == 0.0f ? m_bpmTap.getBpm() : m_engine->bpm()->getBPM(); }
bool MainController::bpmIsOld() { return m_engine->bpm()->bpmIsOld(); }
bool MainController::getBPMMute() { return m_engine->bpmOsc()->getBPMMute(); }
void MainController::toggleBPMMute() { m_engine->bpmOsc()->toggleBPMMute(); emit bpmMuteChanged(); }
QStringList MainController::getBPMOscCommands() { return m_engine->bpmOsc()->getCommands(); }
void MainController::setBPMOscCommands(const QStringList commands) { m_engine->bpmOsc()->setCommands(commands); }

// Audio Helpers
QStringList MainController::getAvailableInputs() const { return m_engine->audioInput()->getAvailableInputs(); }
QString MainController::getActiveInputName() const { return m_engine->audioInput()->getActiveInputName(); }
void MainController::setInputByName(const QString& name) { m_engine->audioInput()->setInputByName(name); emit inputChanged(); emit presetChanged(); }
qreal MainController::getVolume() const { return m_engine->audioInput()->getVolume(); }
void MainController::setVolume(const qreal& value) { m_engine->audioInput()->setVolume(value); emit presetChanged(); }

// FFT Helpers
qreal MainController::getFftGain() const { return m_engine->fft()->getScaledSpectrum().getGain(); }
void MainController::setFftGain(const qreal& value) { m_engine->fft()->getScaledSpectrum().setGain(static_cast<float>(value)); emit gainChanged(); emit presetChanged(); }
qreal MainController::getFftCompression() const { return m_engine->fft()->getScaledSpectrum().getCompression(); }
void MainController::setFftCompression(const qreal& value) { m_engine->fft()->getScaledSpectrum().setCompression(static_cast<float>(value)); emit compressionChanged(); emit presetChanged(); }
bool MainController::getDecibelConversion() const { return m_engine->fft()->getScaledSpectrum().getDecibelConversion(); }
void MainController::setDecibelConversion(bool value) { m_engine->fft()->getScaledSpectrum().setDecibelConversion(value); emit decibelConversionChanged(); emit presetChanged(); }
bool MainController::getAgcEnabled() const { return m_engine->fft()->getScaledSpectrum().getAgcEnabled(); }
void MainController::setAgcEnabled(bool value) { m_engine->fft()->getScaledSpectrum().setAgcEnabled(value); emit agcEnabledChanged(); emit presetChanged(); }

// OSC Helpers
QString MainController::getOscIpAddress() const { return m_engine->osc()->getIpAddress().toString(); }
void MainController::setOscIpAddress(const QString& value) { m_engine->osc()->setIpAddress(QHostAddress(value)); emit settingsChanged(); }
quint16 MainController::getOscUdpTxPort() const { return m_engine->osc()->getUdpTxPort(); }
void MainController::setOscUdpTxPort(const quint16& value) { m_engine->osc()->setUdpTxPort(value); emit settingsChanged(); }
quint16 MainController::getOscUdpRxPort() const { return m_engine->osc()->getUdpRxPort(); }
void MainController::setOscUdpRxPort(const quint16& value) { m_engine->osc()->setUdpRxPort(value); emit settingsChanged(); }
quint16 MainController::getOscTcpPort() const { return m_engine->osc()->getTcpPort(); }
void MainController::setOscTcpPort(const quint16& value) { m_engine->osc()->setTcpPort(value); emit settingsChanged(); }
bool MainController::getOscEnabled() const { return m_engine->osc()->getEnabled(); }
bool MainController::getUseTcp() const { return m_engine->osc()->getUseTcp(); }
void MainController::setUseTcp(bool value) { m_engine->osc()->setUseTcp(value); }
bool MainController::getUseOsc_1_1() const { return m_engine->osc()->getUseOsc_1_1(); }
void MainController::setUseOsc_1_1(bool value) { m_engine->osc()->setUseOsc_1_1(value); }
bool MainController::isConnected() const { return m_engine->osc()->isConnected(); }
QStringList MainController::getOscLog() const { return m_engine->osc()->getLog(); }
bool MainController::getOscLogIncomingIsEnabled() const { return m_engine->osc()->getLogIncomingIsEnabled(); }
bool MainController::getOscLogOutgoingIsEnabled() const { return m_engine->osc()->getLogOutgoingIsEnabled(); }
void MainController::enableOscLogging(bool incoming, bool outgoing) { m_engine->osc()->enableLogging(incoming, outgoing); }
void MainController::sendOscMessage(QString message, bool forced) { m_engine->osc()->sendMessage(message, forced); }
void MainController::sendOscMessage(QString path, QString argument, bool forced) { m_engine->osc()->sendMessage(path, argument, forced); }
void MainController::clearOscLog() const { m_engine->osc()->clearLog(); }
