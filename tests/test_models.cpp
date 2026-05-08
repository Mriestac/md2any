#include "conversion_result.h"
#include "conversion_task.h"

#include <QtTest/QtTest>

using namespace Md2Any;

class ModelsTest : public QObject
{
    Q_OBJECT

private slots:
    void conversionTaskHasStableDefaults()
    {
        const ConversionTask task;

        QCOMPARE(task.outputFormat, QStringLiteral("html"));
        QCOMPARE(task.overwrite, false);
        QVERIFY(task.inputPath.isEmpty());
        QVERIFY(task.outputPath.isEmpty());
        QVERIFY(task.extraArgs.isEmpty());
    }

    void supportedOutputFormatsAreCentralized()
    {
        const auto formats = supportedOutputFormats();

        QCOMPARE(formats, QStringList({QStringLiteral("html"), QStringLiteral("docx"), QStringLiteral("pdf")}));
        QVERIFY(isSupportedOutputFormat(QStringLiteral("html")));
        QVERIFY(isSupportedOutputFormat(QStringLiteral("DOCX")));
        QVERIFY(isSupportedOutputFormat(QStringLiteral(" pdf ")));
        QVERIFY(!isSupportedOutputFormat(QStringLiteral("epub")));
        QVERIFY(!isSupportedOutputFormat(QString()));
    }

    void conversionResultHasFailureDefaults()
    {
        const ConversionResult result;

        QCOMPARE(result.success, false);
        QCOMPARE(result.exitCode, -1);
        QVERIFY(result.command.isEmpty());
        QVERIFY(result.standardOutput.isEmpty());
        QVERIFY(result.standardError.isEmpty());
        QVERIFY(result.outputPath.isEmpty());
        QVERIFY(result.message.isEmpty());
    }
};

QTEST_MAIN(ModelsTest)

#include "test_models.moc"
