#include <QTest>
#include <QException>
#include "snapshottesting.h"

SnapshotTesting::Test::Test()
{

}

QString SnapshotTesting::Test::name() const
{
    return m_name;
}

void SnapshotTesting::Test::setName(const QString &name)
{
    m_name = name;
}

QString SnapshotTesting::Test::suffix() const
{
    return m_suffix;
}

void SnapshotTesting::Test::setSuffix(const QString &suffix)
{
    m_suffix = suffix;
}

QString SnapshotTesting::Test::capture(QObject *object, SnapshotTesting::CaptureOptions options)
{
    return SnapshotTesting::capture(object, options);
}

bool SnapshotTesting::Test::match(const QString &snapshot, const QImage screenshot)
{
    QString realName = m_name;
    if (!m_suffix.isNull()) {
        realName  += m_suffix;
    }

    return SnapshotTesting::matchStoredSnapshot(realName, snapshot, screenshot);
}
