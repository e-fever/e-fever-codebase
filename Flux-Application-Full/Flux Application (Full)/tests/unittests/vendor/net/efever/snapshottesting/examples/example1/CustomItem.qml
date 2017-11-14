import QtQuick 2.0
import QtQuick.Layouts 1.3

Item {
    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#000000"
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#FF0000"
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#FFCC00"
        }

    }
}
