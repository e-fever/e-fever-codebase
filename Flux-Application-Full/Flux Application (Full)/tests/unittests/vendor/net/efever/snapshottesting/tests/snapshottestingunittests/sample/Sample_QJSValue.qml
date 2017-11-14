// Test QJSValue property
import QtQuick 2.0

Item {

    property var value1: 1

    property var value2: "2"

    property var value3: ({
                              value1: 1,
                              value2: "2"
                          })

    property var value4: [ 1, 2, 3]
}
