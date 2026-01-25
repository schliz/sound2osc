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


// ------------- Dark styled TabBar (Qt6) -----------------
// Note: Qt6 removed TabView. Use TabBar + StackLayout instead.
// This component provides a styled TabBar; combine with StackLayout for content.
TabBar {
    id: control

    background: Rectangle {
        color: "transparent"
    }

    contentItem: ListView {
        model: control.contentModel
        currentIndex: control.currentIndex
        spacing: 2
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightMoveDuration: 0
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 40
        preferredHighlightEnd: width - 40
    }

    // Custom delegate for TabButton styling
    delegate: TabButton {
        id: tabButton
        width: Math.max(implicitWidth, 80)
        height: 30

        contentItem: Text {
            text: tabButton.text
            font.pointSize: 10
            color: "#B5B7BA"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            color: tabButton.checked ? "#1C2C40" : "#333333"
            border.color: "#B5B7BA"
            border.width: 1
            radius: 2
        }
    }
}
