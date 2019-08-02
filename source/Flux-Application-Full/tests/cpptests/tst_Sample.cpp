#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class Sample : public QObject
{
    Q_OBJECT

public:
    Sample();
    ~Sample();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

Sample::Sample()
{

}

Sample::~Sample()
{

}

void Sample::initTestCase()
{

}

void Sample::cleanupTestCase()
{

}

void Sample::test_case1()
{

}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    Sample tc;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_Sample.moc"
