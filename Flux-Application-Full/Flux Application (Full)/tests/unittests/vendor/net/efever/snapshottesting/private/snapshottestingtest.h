#ifndef SNAPSHOTTESTINGTEST_H
#define SNAPSHOTTESTINGTEST_H

#include <QString>
#include <QObject>
#include <QImage>
#include "private/snapshottestingoptions.h"

namespace SnapshotTesting {

    class Test
    {
    public:
        Test();

        QString name() const;

        void setName(const QString &name);

        QString suffix() const;

        void setSuffix(const QString &suffix);

        QString capture(QObject* object, SnapshotTesting::CaptureOptions options = SnapshotTesting::CaptureOptions());

        bool match(const QString& snapshot, const QImage screenshot = QImage());

    private:

        QString m_name;

        QString m_suffix;
    };

}

#endif // SNAPSHOTTESTINGTEST_H
