#include "conversion_process.h"

#include <QCoreApplication>
#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Md2Any;

Q_DECLARE_METATYPE(Md2Any::ConversionProcessResult)

class ConversionProcessTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qRegisterMetaType<Md2Any::ConversionProcessResult>();
    }

    void fakePandocSuccessCapturesOutput()
    {
        ConversionProcess process;
        QSignalSpy finishedSpy(&process, SIGNAL(finished(Md2Any::ConversionProcessResult)));

        process.start(fakePandocPath(), {
            QStringLiteral("-t"),
            QStringLiteral("html"),
            QStringLiteral("input.md"),
            QStringLiteral("-o"),
            QDir::temp().filePath(QStringLiteral("md2any-process-success.html")),
        });

        if (finishedSpy.isEmpty()) {
            QVERIFY(finishedSpy.wait(3000));
        }
        const auto result = finishedSpy.takeFirst().at(0).value<ConversionProcessResult>();

        QCOMPARE(result.exitCode, 0);
        QVERIFY(result.standardError.isEmpty());
        QVERIFY(!result.canceled);
    }

    void fakePandocFailureCapturesStderr()
    {
        ConversionProcess process;
        QSignalSpy finishedSpy(&process, SIGNAL(finished(Md2Any::ConversionProcessResult)));

        process.start(fakePandocPath(), {QStringLiteral("--fail")});

        if (finishedSpy.isEmpty()) {
            QVERIFY(finishedSpy.wait(3000));
        }
        const auto result = finishedSpy.takeFirst().at(0).value<ConversionProcessResult>();

        QCOMPARE(result.exitCode, 7);
        QVERIFY(result.standardError.contains(QStringLiteral("fake pandoc failure")));
        QVERIFY(!result.canceled);
    }

    void missingProgramReturnsError()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ConversionProcess process;
        QSignalSpy finishedSpy(&process, SIGNAL(finished(Md2Any::ConversionProcessResult)));

        process.start(tempDir.filePath(QStringLiteral("missing-pandoc.exe")), {});

        if (finishedSpy.isEmpty()) {
            QVERIFY(finishedSpy.wait(3000));
        }
        const auto result = finishedSpy.takeFirst().at(0).value<ConversionProcessResult>();

        QVERIFY(!result.errorString.isEmpty());
    }

private:
    static QString fakePandocPath()
    {
        return QCoreApplication::applicationDirPath() + QStringLiteral("/fake_pandoc.exe");
    }
};

QTEST_MAIN(ConversionProcessTest)

#include "test_conversion_process.moc"
