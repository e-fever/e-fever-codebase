import QtQuick 2.0

Item {
    id: component

    property alias source: image.source

    property alias sourceSize: image.sourceSize

    property alias imageWidth: image.width
    property alias imageHeight: image.height

    Image {
        id: image
        anchors.centerIn: parent

        function refresh() {
            if (image.sourceSize.width === 0 || image.sourceSize.height === 0) {
                image.scale = 1;
            }

            var sw = component.width / image.sourceSize.width;
            var sh = component.height / image.sourceSize.height;

            image.scale = Math.min(sw, sh, 1);
        }

        onWidthChanged: refresh();
        onHeightChanged: refresh();
    }
}
