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

#include "TriggerGuiController.h"

#include <sound2osc/dsp/FFTAnalyzer.h>
#include <sound2osc/dsp/ScaledSpectrum.h>

TriggerGuiController::TriggerGuiController(TriggerGenerator *trigger, QObject *parent)
    : QObject(parent)
    , m_trigger(trigger)
{
    // connect on and off signals of TriggerFilter with the signals of this controller:
    connect(&(trigger->getTriggerFilter()), &TriggerFilter::onSignalSent, this, &TriggerGuiController::triggerOn);
	connect(&(trigger->getTriggerFilter()), &TriggerFilter::offSignalSent, this, &TriggerGuiController::triggerOff);
	connect(&(trigger->getTriggerFilter()), &TriggerFilter::onSignalSent, this, &TriggerGuiController::activeChanged);
	connect(&(trigger->getTriggerFilter()), &TriggerFilter::offSignalSent, this, &TriggerGuiController::activeChanged);
}

void TriggerGuiController::resetParameters()
{
	m_trigger->resetParameters();
	emit parameterChanged();
	emit oscLabelTextChanged();
}

qreal TriggerGuiController::getMidFreqNormalized() const
{
	// TODO: remove dependency from ScaledSpectrum constants
	ScaledSpectrum spectrum(SCALED_SPECTRUM_BASE_FREQ, SCALED_SPECTRUM_LENGTH);
	int index = spectrum.getIndexForFreq(m_trigger->getMidFreq());
	qreal normalized = index / qreal(SCALED_SPECTRUM_LENGTH);
	return normalized;
}

void TriggerGuiController::setMidFreqNormalized(const qreal &value)
{
	// TODO: remove dependency from ScaledSpectrum constants
	ScaledSpectrum spectrum(SCALED_SPECTRUM_BASE_FREQ, SCALED_SPECTRUM_LENGTH);
	double freq = spectrum.getFreqAtPosition(value);
	setMidFreq(static_cast<int>(freq));
}
