import QtQuick 2.0
import QtQuick.Layouts 1.1

Grid {
    rows: 2
    columns: 2
    spacing: 10

    Rectangle {
        color: "red"
        width: 50
        height: 60
    }

    Rectangle {
        color: "blue"
        width: 40
        height: 70
    }

    RowLayout {
        width: 200
        height: 30

        Rectangle {
            color: "green"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Rectangle {
            color: "white"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Rectangle {
            color: "green"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }


}
