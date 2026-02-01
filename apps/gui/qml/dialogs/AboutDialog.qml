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
import QtQuick.Dialogs

import "style"  // import all files in style dir

// ---------------------------- About Dialog ----------------------
Dialog {
	id: dialog
	title: "About"

	contentItem: Item {
		implicitWidth: 600
		implicitHeight: 400

		DarkBlueStripedBackground {}

		// ------------------------ Top Area with Text and Logo ----------------
		Column {
			anchors.margins: 20
			anchors.fill: parent

			Row {
				width: parent.width
				height: parent.height - 40

				// ------------------------ Left Area with Text --------------------
				Column {
					id: textArea
					height: parent.height
					width: parent.width - logoArea.width

					GreyText {
						width: parent.width
						height: 40
						font.pointSize: 14
						text: "sound2osc"
					}
					GreyText {
						width: parent.width
						height: 30
						font.pointSize: 10
						text: "Version: " + controller.getVersionString()
					}
				}

				// ------------------------- App Logo on the right ---------------------
				Item {
					id: logoArea
					height: parent.height
					width: 200
					Image {
						source: "qrc:/images/icons/logo.png"
						width: 200
						height: 200
						anchors.centerIn: parent
					}
				}
			}

			// --------------- Bottom Area with OK Button and Copyright notes -------------
			Row {
				id: bottomRow
				width: parent.width
				height: 40
				GreyText {
					height: parent.height
					width: parent.width * 0.4
                    // Spacer
				}
				DarkButton {
					height: parent.height
					width: parent.width * 0.2
					text: "OK"
					onClicked: {
						dialog.close()
						controller.dialogIsClosed(dialog)
					}
				}
				GreyText {
					height: parent.height
					width: parent.width * 0.4
					text: "© ETC Inc. / Christian Schliz"
					verticalAlignment: Text.AlignBottom
					horizontalAlignment: Text.AlignHCenter
				}

			}
		}
	}
}
