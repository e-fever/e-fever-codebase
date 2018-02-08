#pragma once

#include <QString>
#include <QVariantMap>
#include <private/snapshottestingoptions.h>
#include <private/snapshottestingrenderer.h>
#include <private/snapshottestingtest.h>

namespace SnapshotTesting {

    Test createTest();

    /// Set the file name to save the stored snapshots
    void setSnapshotsFile(const QString& file);

    /// Get the file name of stored snapshots
    QString snapshotsFile();

    void setScreenshotImagePath(const QString& path);

    /// Load "snapshots" from the "snapshotsFile"
    QVariantMap loadStoredSnapshots();

    /// Run QRegExp replace on everything line of the input
    QString replaceLines(const QString& input, QRegExp regexp, QString replace);

    void saveSnapshots();

    void setSnapshotText(const QString& name , const QString& content);

    void setInteractiveEnabled(bool value);

    bool interactiveEnabled();

    void setIgnoreAllMismatched(bool value);

    bool ignoreAllMismatched();

    QString diff(QString original, QString current);

    QString capture(QObject* object, CaptureOptions options = CaptureOptions());

    bool matchStoredSnapshot(const QString& name, const QString& snapshot);

    bool matchStoredSnapshot(const QString& name, const QString& snapshot, const QImage& screenshot);

    /// Compare the input snapshot and stored snapshot. Returns true if they are matched. Otherwise, it is fale. Unlike matchStoredSnapshot(), it won't trigger UI.
    bool tryMatchStoredSnapshot(const QString& name, const QString& snapshot);

    void addClassIgnoredProperty(const QString &className , const QString& property);

    void removeClassIgnoredProperty(const QString &className , const QString& property);

    void addComponentIgnoreProperty(const QString &componentName , const QString& package, const QString& property);

    void removeComponentIgnoreProperty(const QString &componentName , const QString& package, const QString& property);

}
