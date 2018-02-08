import QtQuick 2.6
import QtQuick.Controls 1.4

Item {
    id: contentView
    implicitWidth: 640
    implicitHeight: 480

    property string diff: ""
    property string originalVersion: ""
    property string currentVersion: ""

    property string screenshot: ""
    property string previousScreenshot: ""

    property string combinedScreenshot: ""
    property string monospaceFont: ""

    property alias tabIndex: tabView.currentIndex

    TabView {
        id: tabView
        anchors.fill: parent
        anchors.bottomMargin: 40
        clip: true

        Tab {
            title: "Diff"
            Item {
                TextArea {
                    font.family: monospaceFont
                    anchors.fill: parent
                    text: diff
                }
            }
        }

        Tab {
            title: "Stored Snapshot"
            Item {
                TextArea {
                    font.family: monospaceFont
                    anchors.fill: parent
                    text: originalVersion
                }
            }
        }

        Tab {
            title: "Current Snapshot"
            Item {
                TextArea {
                    font.family: monospaceFont
                    anchors.fill: parent
                    text: currentVersion
                }
            }
        }
    }

    Component {
        id: screenshotViewer
        Item {

            ScreenshotBrowser {
                anchors.fill: parent
                anchors.margins: 4
                screenshot: contentView.screenshot
                previousScreenshot: contentView.previousScreenshot
                combinedScreenshot: contentView.combinedScreenshot
            }
        }
    }

    onScreenshotChanged: {
        if (screenshot  === "") {
            return;
        }

        tabView.addTab("Screenshot", screenshotViewer);
    }

    Text {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 40
        verticalAlignment: Text.AlignVCenter

        leftPadding: 10
        rightPadding: 10
        text: qsTr("New snapshot does not match the stored snapshot. Inspect your code and press \\"Yes\\" to update the changes, or press \\"No\\" to reject.")

        wrapMode: Text.WordWrap

    }


}
