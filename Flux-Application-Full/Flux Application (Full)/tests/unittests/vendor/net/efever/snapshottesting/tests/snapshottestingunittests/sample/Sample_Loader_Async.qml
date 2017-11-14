import QtQuick 2.0

Item {
    id: sampleLoaderAsync

    Loader {
        id: loader

        asynchronous: true

        source: Qt.resolvedUrl("./Sample1.qml")
    }

    Image {
        id: image

        asynchronous: true
        cache: false
        source: "https://github.com/benlau/junkcode/blob/master/docs/Lanto%20-%20Screenshot%201.png?raw=true"
    }

}
