/* Sample3 - Test Repeater
 */
import QtQuick 2.0
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 640
    height: 480

    Column {
        id: column
        Repeater {
            model: 5
            delegate: Item {
                id: repeaterItem
                width: 640
                height: 48
            }
        }
    }

    ListView {
        model: 5
        delegate: Item {
            id: listViewItem
            width: 640
            height: 48
        }
    }

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: 120
        }
    }
}
