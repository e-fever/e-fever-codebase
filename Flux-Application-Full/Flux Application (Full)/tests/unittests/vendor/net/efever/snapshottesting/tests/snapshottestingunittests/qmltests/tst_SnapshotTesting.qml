import QtQuick 2.0
import QtTest 1.0
import SnapshotTesting 1.0
import "../sample"

Item {
    width: 640
    height: 480

    Sample1 {
        id: sample1
    }

    Sample2 {
        id: sample2
    }

    Sample5 {
        id: sample5
    }

    Sample6 {
        id: sample6
    }

    Sample7 {
        id: sample7
    }

    Sample8 {
        id: sample8
    }

    Sample9 {
        id: sample9
    }

    Sample_QJSValue {
        id: sample_qjsvalue
    }

    Column {
        id: column
    }

    TestCase {
        name: "SnapshotTesting"
        when: windowShown

        function test_capture() {
            var objects = {
                "sample1": sample1,
                "sample2": sample2,
                "sample5": sample5,
                "sample6": sample6,
                "sample7": sample7,
                "sample8": sample8,
                "sample9": sample9,
                "sample_qjsvalue": sample_qjsvalue,
                "column": column,
            }

            for (var name in objects) {
                var target = objects[name];
                var snapshot;
                snapshot = SnapshotTesting.capture(target, {indentSize: 4});
                snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
                SnapshotTesting.matchStoredSnapshot("qml_test_capture_" + name, snapshot);

                snapshot = SnapshotTesting.capture(target, {expandAll: true});
                snapshot = snapshot.replace(new RegExp(Qt.resolvedUrl(".."), "g"), "");
                SnapshotTesting.matchStoredSnapshot("qml_test_capture_expandAll_" + name, snapshot);
            }
        }

        function test_caller() {
            compare(SnapshotTesting._caller(), "test_caller");
        }
    }



}
