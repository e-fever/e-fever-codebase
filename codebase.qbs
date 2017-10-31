import qbs
import qbs.File

Project {
    id: project

    references: qbsFinder.qbsFiles

    Probe {
        id: qbsFinder

        property var qbsFiles: []

        configure: {
            var files = [];
            var maxdepth = 2;

            function find(path, depth) {
                console.info(path);
                var file = path + "/" + "qtcwizard.qbs"
                if (File.exists(file)) {
                    files.push(file);
                    return;
                }

                if (depth < maxdepth) {
                    var dirs = File.directoryEntries(path,File.Dirs);

                    for (var i in dirs) {
                        var dir = dirs[i];
                        if (dir === "." || dir === "..") {
                            continue;
                        }
                        find(path +"/" + dir, depth + 1);
                    }
                }
            }

            find(path, 1);

            console.info(JSON.stringify(files));
            qbsFiles = files;
            found = true;
        }

    }
}
