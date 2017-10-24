import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QuickFlux 1.1
import MYPACKAGE.constants 1.0
import MYPACKAGE.actions 1.0
import MYPACKAGE.stores 1.0
import MYPACKAGE.middlewares 1.0

Window {
    id: mainWindow
    visible: true
    width: 480
    height: 640

    MouseArea {
        anchors.fill: parent
        onClicked: {
            AppActions.quitApp();
        }
    }

    Text {
        text: qsTr(MainStore.text)
        anchors.centerIn: parent
    }

}
