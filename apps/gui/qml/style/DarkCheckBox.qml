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

import QtQuick
import QtQuick.Controls


// ------------- Dark styled CheckBox (Qt6) -----------------
CheckBox {
	id: control
	property int fontSize: 10
	property color textColor: "#b5b7ba"
	property color disabledTextColor: "#666"

	indicator: Rectangle {
		implicitWidth: 16
		implicitHeight: 16
		x: control.leftPadding
		y: parent.height / 2 - height / 2
		radius: 3
		border.color: control.activeFocus ? "darkblue" : (control.enabled ? "gray" : "#555")
		border.width: 1
		gradient: Gradient {
			GradientStop { position: 0 ; color: control.pressed ? "#444" : "#333" }
			GradientStop { position: 1 ; color: control.pressed ? "#555" : "#444" }
		}
		Rectangle {
			visible: control.checked
			color: "lightgreen"
			border.color: "#FFF"
			radius: 1
			anchors.margins: 3
			anchors.fill: parent
		}
	}

	contentItem: Text {
		text: control.text
		font.pointSize: control.fontSize
		color: control.enabled ? control.textColor : control.disabledTextColor
		verticalAlignment: Text.AlignVCenter
		leftPadding: control.indicator.width + control.spacing
	}
}
