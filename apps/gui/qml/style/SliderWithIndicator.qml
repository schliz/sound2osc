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


// ------------- Dark styled Slider with optional level indicator (Qt6) -----------------
Slider {
    id: control
    property real indicator: 0.0
    property bool showIndicator: true
    property color handleActiveColor: "#B5B7BA"
    // Qt5 compatibility aliases
    property alias minimumValue: control.from
    property alias maximumValue: control.to
    orientation: Qt.Vertical
    from: 0.0
    to: 1.0

    background: Rectangle {
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        implicitWidth: control.horizontal ? 200 : 8
        implicitHeight: control.horizontal ? 8 : 200
        width: control.horizontal ? control.availableWidth : implicitWidth
        height: control.horizontal ? implicitHeight : control.availableHeight
        color: "#333333"
        radius: 8

        // Indicator/value fill
        Rectangle {
            width: control.horizontal
                   ? (showIndicator ? parent.width * indicator : control.visualPosition * parent.width)
                   : parent.width
            height: control.horizontal
                    ? parent.height
                    : (showIndicator ? parent.height * indicator : (1.0 - control.visualPosition) * parent.height)
            y: control.horizontal ? 0 : parent.height - height
            color: control.enabled ? "#B5B7BA" : "#777"
            radius: 8
        }
    }

    handle: Rectangle {
        x: control.leftPadding + (control.horizontal
           ? control.visualPosition * (control.availableWidth - width)
           : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal
           ? (control.availableHeight - height) / 2
           : control.visualPosition * (control.availableHeight - height))
        implicitWidth: 18
        implicitHeight: 18
        radius: 9
        color: (!showIndicator || indicator >= control.value) ? handleActiveColor : "#555"
        border.color: "gray"
        border.width: 2
        visible: control.enabled
    }
}
