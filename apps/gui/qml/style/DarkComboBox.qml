// Copyright (c) 2016 Electronic Theatre Controls, Inc., http://www.etcconnect.com
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


// ------------- Dark styled ComboBox (Qt6) -----------------
ComboBox {
	id: control

	background: Rectangle {
		implicitWidth: 120
		implicitHeight: 30
		border.width: 2
		border.color: "#888"
		gradient: Gradient {
			GradientStop { position: 0 ; color: control.pressed ? "#444" : "#333" }
			GradientStop { position: 1 ; color: control.pressed ? "#555" : "#444" }
		}
	}

	indicator: Text {
		text: "▼"
		color: control.enabled ? "#B5B7BA" : "#666666"
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter
		anchors.rightMargin: 5
		font.pointSize: 10
	}

	contentItem: Text {
		leftPadding: 5
		rightPadding: control.indicator.width + control.spacing
		text: control.displayText
		font.pointSize: 10
		color: control.enabled ? "#888" : "#666"
		verticalAlignment: Text.AlignVCenter
		elide: Text.ElideRight
	}

	popup: Popup {
		y: control.height
		width: control.width
		implicitHeight: contentItem.implicitHeight
		padding: 1

		contentItem: ListView {
			clip: true
			implicitHeight: contentHeight
			model: control.popup.visible ? control.delegateModel : null
			currentIndex: control.highlightedIndex
			ScrollIndicator.vertical: ScrollIndicator { }
		}

		background: Rectangle {
			border.color: "#888"
			color: "#333"
		}
	}

	delegate: ItemDelegate {
		width: control.width
		contentItem: Text {
			text: modelData
			color: "#b5b7ba"
			font.pointSize: 10
			elide: Text.ElideRight
			verticalAlignment: Text.AlignVCenter
		}
		background: Rectangle {
			color: highlighted ? "#555" : "#333"
		}
		highlighted: control.highlightedIndex === index
	}
}
