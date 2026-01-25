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


// ------------- Dark styled SpinBox (Qt6) -----------------
SpinBox {
    id: control
    font.pointSize: 10

    contentItem: TextInput {
        z: 2
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        color: "#B5B7BA"
        selectionColor: "#1C2C40"
        selectedTextColor: "#B5B7BA"
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
    }

    up.indicator: Rectangle {
        x: control.mirrored ? 0 : parent.width - width
        height: parent.height
        implicitWidth: 30
        implicitHeight: 30
        color: control.up.pressed ? "#1C2C40" : "#333333"
        border.color: "#444"
        border.width: 1

        Text {
            text: "+"
            font.pixelSize: control.font.pixelSize * 1.5
            color: control.up.pressed ? "#fff" : "#B5B7BA"
            anchors.centerIn: parent
        }
    }

    down.indicator: Rectangle {
        x: control.mirrored ? parent.width - width : 0
        height: parent.height
        implicitWidth: 30
        implicitHeight: 30
        color: control.down.pressed ? "#1C2C40" : "#333333"
        border.color: "#444"
        border.width: 1

        Text {
            text: "-"
            font.pixelSize: control.font.pixelSize * 1.5
            color: control.down.pressed ? "#fff" : "#B5B7BA"
            anchors.centerIn: parent
        }
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 30
        color: "#333333"
        border.color: "#444"
        border.width: 1
    }
}
