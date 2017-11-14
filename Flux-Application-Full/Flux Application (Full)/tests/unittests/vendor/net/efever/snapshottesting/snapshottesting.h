#pragma once

#include <QString>
#include <QVariantMap>

namespace SnapshotTesting {

    class Options {
    public:

        inline Options() {
            captureVisibleItemOnly = true;
            expandAll = false;
            hideId = false;
            indentSize = 4;
            captureWhenLoaded = true;
        }

        bool captureVisibleItemOnly;
        bool expandAll;
        bool hideId;
        int indentSize;

        /// Capture only if the component is loaded completely
        bool captureWhenLoaded;
    };

    /// Set the file name to save the stored snapshots
    void setSnapshotsFile(const QString& file);

    QString snapshotsFile();

    /// Load "snapshots" from the "snapshotsFile"
    QVariantMap loadStoredSnapshots();

    void saveSnapshots();

    void setSnapshot(const QString& name , const QString& content);

    void setInteractiveEnabled(bool value);

    bool interactiveEnabled();

    void setIgnoreAllMismatched(bool value);

    bool ignoreAllMismatched();

    QString diff(QString original, QString current);

    QString capture(QObject* object, Options options = Options());

    bool matchStoredSnapshot(const QString& name, const QString& snapshot);

    void addClassIgnoredProperty(const QString &className , const QString& property);

    void removeClassIgnoredProperty(const QString &className , const QString& property);
}
