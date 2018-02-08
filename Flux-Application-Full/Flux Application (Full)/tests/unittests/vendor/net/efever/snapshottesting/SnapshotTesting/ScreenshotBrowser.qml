import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Item {
    id: component

    property string screenshot: ""

    property string previousScreenshot: ""

    property string combinedScreenshot: ""

    function showCombinedScreenshot() {
        combinedButton.checked = true;
    }

    function base64(data) {
        if (data === "") {
            return "";
        }
        return "data:image/png;base64," + data;
    }

    ScaleToFitImage {
        id: singleImage
        anchors.fill: parent
        source: "data:image/png;base64," + screenshot
    }

    ColumnLayout {
        id: dualImage
        anchors.fill: parent
        visible: false
        enabled: false

        Row {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: 40

            ExclusiveGroup { id: displayMode }

            RadioButton {
                id: sideBySideButton
                text: "Side by Side"
                checked: true
                exclusiveGroup: displayMode
                width: 120
                height: 20
            }

            RadioButton {
                id: overlappedButton
                text: "Overlapped"
                exclusiveGroup: displayMode
                width: 120
                height: 20
            }

            RadioButton {
                id: combinedButton
                text: "Combined"
                objectName: "CombinedButton"
                checked: false
                visible: combinedScreenshot !== ""
                exclusiveGroup: displayMode
                width: 120
                height: 20
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                id: sideBySideView
                visible: sideBySideButton.checked
                enabled: visible
                anchors.fill: parent

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    ScaleToFitImage {
                        anchors.fill: parent
                        anchors.margins: 4
                        source: {
                            if (previousScreenshot === "") {
                                return "";
                            }
                            return "data:image/png;base64," + previousScreenshot;
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    ScaleToFitImage {
                        anchors.fill: parent
                        anchors.margins: 4
                        source: "data:image/png;base64," + screenshot
                    }
                }
            }

            Item {
                id: overlappedView
                visible: overlappedButton.checked
                enabled: visible
                anchors.fill: parent
                anchors.margins: 4
                ScaleToFitImage {
                    x: 0
                    y: 0
                    width: parent.width
                    height: parent.height
                    opacity: 0.5
                    source: base64(previousScreenshot)

                    MouseArea {
                        anchors.fill: parent
                        drag.target: parent
                    }
                }

                ScaleToFitImage {
                    x: 0
                    y: 0
                    width: parent.width
                    height: parent.height
                    opacity: 0.5
                    source: base64(screenshot)

                    MouseArea {
                        anchors.fill: parent
                        drag.target: parent
                    }
                }
            }

            ScaleToFitImage {
                anchors.fill: parent
                anchors.margins: 4
                visible: combinedButton.checked
                enabled: visible
                source: {
                    if (combinedScreenshot === "") {
                        return "";
                    }
                    return "data:image/png;base64," + combinedScreenshot;
                }
            }
        }
    }

    states: [
        State {
            name: "SingleMode"
        },
        State {
            name: "DualMode"
            when: previousScreenshot !== ""

            PropertyChanges {
                target: singleImage
                visible: false
            }

            PropertyChanges {
                target: dualImage
                visible: true
                enabled: true
            }
        },
        State {
            name: "CombinedMode"
        }
    ]

}
