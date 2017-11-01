import QtQuick 2.3
import QtQuick.Window 2.2
import QtMultimedia 5.5
import QtQuick.Controls 1.4
import PACKAGE.constants 1.0
import PACKAGE.actions 1.0
import PACKAGE.stores 1.0
import PACKAGE.middlewares 1.0
import QuickFlux 1.1

Window {
    id: mainWindow
    visible: false
    width: 480
    height: 640

    MiddlewareList {
        applyTarget: AppDispatcher
        SystemMiddleware {
            mainWindow: mainWindow
        }
    }

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
