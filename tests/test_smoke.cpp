#include <QtTest/QtTest>

class SmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void projectSkeletonIsTestable()
    {
        QVERIFY(true);
    }
};

QTEST_MAIN(SmokeTest)

#include "test_smoke.moc"
