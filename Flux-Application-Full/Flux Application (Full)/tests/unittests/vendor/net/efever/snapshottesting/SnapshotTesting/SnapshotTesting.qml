pragma Singleton
import QtQuick 2.0
import SnapshotTesting.Private 1.0

Item {
    id: snapshotTesting

    property string snapshotsFile: Adapter.snapshotsFile

    function capture(object, options) {
        return Adapter.capture(object, options);
    }

    function matchStoredSnapshot(name, snapshot) {
        if (!Adapter.matchStoredSnapshot(name,snapshot)) {
            throw new Error("matchStoredSnapshot: Current snapshot does not match with stored snapshot");
        }
    }

    function _caller() {
        try {
            throw new Error();
        } catch (e) {

            var lines = e.stack.split("\\n");
            var line = lines[1];
            var token = line.split("@");

            return token[0];
        }

    }

    onSnapshotsFileChanged: {
        if (Adapter.snapshotsFile !== snapshotTesting.snapshotsFile) {
            Adapter.snapshotsFile = snapshotTesting.snapshotsFile;
        }
    }

}
