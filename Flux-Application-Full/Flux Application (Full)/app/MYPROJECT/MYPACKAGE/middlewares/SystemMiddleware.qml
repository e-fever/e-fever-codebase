import QtQuick 2.7
import QuickFlux 1.1
import "../actions"

Middleware {

    function dispatch(type, message) {

        if (type === ActionTypes.quitApp) {
            Qt.quit();
        }

        next(type, message);
    }

}

