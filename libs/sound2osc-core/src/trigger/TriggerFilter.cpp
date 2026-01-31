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

#include <sound2osc/trigger/TriggerFilter.h>

#include <sound2osc/osc/OSCNetworkManager.h>

#include <QDebug>
#include <QTimer>

TriggerFilter::TriggerFilter(OSCNetworkManager* osc, TriggerOscParameters& oscParameters, bool mute)
	: QObject(0)
    , m_mute(mute)
	, m_onDelay(0.0)
	, m_offDelay(0.0)
	, m_maxHold(0.0)
	, m_outputIsActive(false)
	, m_osc(osc)
	, m_oscParameters(oscParameters)
{
	m_onDelayTimer.setSingleShot(true);
	connect(&m_onDelayTimer, &QTimer::timeout, this, &TriggerFilter::onOnDelayEnd);
	m_maxHoldTimer.setSingleShot(true);
	connect(&m_maxHoldTimer, &QTimer::timeout, this, &TriggerFilter::onMaxHoldEnd);
	m_offDelayTimer.setSingleShot(true);
	connect(&m_offDelayTimer, &QTimer::timeout, this, &TriggerFilter::onOffDelayEnd);
}

void TriggerFilter::triggerOn()
{
	// stop releaseDelayTimer if it is running:
	m_offDelayTimer.stop();

	// ignore triggerOn if output is still active:
	if (m_outputIsActive) return;

	// ignore triggerOn if onDelayTimer of previous triggerOn is still running:
	if (m_onDelayTimer.isActive()) return;

	// call onOnDelayEnd() after onDelay time:
	m_onDelayTimer.start(static_cast<int>(m_onDelay * 1000));
}

void TriggerFilter::triggerOff()
{
	// stop onDelayTimer if it is running:
	m_onDelayTimer.stop();

	// ignore triggerOff if output is not active:
	if (!m_outputIsActive) return;

	// ignore triggerOff if offDelayTimer of previous triggerOff is still running:
	if (m_offDelayTimer.isActive()) return;

	// call onOffDelayEnd() after offDelay time:
	m_offDelayTimer.start(static_cast<int>(m_offDelay * 1000));
}

void TriggerFilter::sendOnSignal()
{
	QString message = m_oscParameters.getOnMessage();
    if (!message.isEmpty() && !m_mute)	m_osc->sendMessage(message);
	emit onSignalSent();
}

void TriggerFilter::sendOffSignal()
{
	QString message = m_oscParameters.getOffMessage();
    if (!message.isEmpty() && !m_mute) m_osc->sendMessage(message);
	emit offSignalSent();
}

void TriggerFilter::save(const QString name, QSettings &settings) const
{
	settings.setValue(name + "/onDelay", m_onDelay);
	settings.setValue(name + "/offDelay", m_offDelay);
	settings.setValue(name + "/maxHold", m_maxHold);
}

void TriggerFilter::restore(const QString name, QSettings &settings)
{
	setOnDelay(settings.value(name + "/onDelay").toReal());
	setOffDelay(settings.value(name + "/offDelay").toReal());
	setMaxHold(settings.value(name + "/maxHold").toReal());
}

QJsonObject TriggerFilter::toState() const
{
    QJsonObject state;
    state["onDelay"] = m_onDelay;
    state["offDelay"] = m_offDelay;
    state["maxHold"] = m_maxHold;
    return state;
}

void TriggerFilter::fromState(const QJsonObject& state)
{
    setOnDelay(state["onDelay"].toDouble(0.0));
    setOffDelay(state["offDelay"].toDouble(0.0));
    setMaxHold(state["maxHold"].toDouble(0.0));
}

void TriggerFilter::onOnDelayEnd()
{
	Q_ASSERT(!m_outputIsActive);

	m_outputIsActive = true;
	sendOnSignal();

	// if maxHold is set, call onMaxHoldEnd after maxHold time:
	if (m_maxHold > 0) {
		m_maxHoldTimer.start(static_cast<int>(m_maxHold * 1000));
	}
}

void TriggerFilter::onMaxHoldEnd()
{
	Q_ASSERT(m_outputIsActive);

	m_outputIsActive = false;
	sendOffSignal();

	// if offDelayTimer is running, stop it:
	m_offDelayTimer.stop();
}

void TriggerFilter::onOffDelayEnd()
{
	Q_ASSERT(m_outputIsActive);

	m_outputIsActive = false;
	sendOffSignal();

	// if maxHoldTimer is running, stop it:
	m_maxHoldTimer.stop();
}
