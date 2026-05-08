#include "pandoc_formats.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Md2Any;

class PandocFormatsTest : public QObject
{
    Q_OBJECT

private slots:
    void mergesReadableAndWritableFormats()
    {
        const auto formats = mergePandocFormats(
            {QStringLiteral("markdown"), QStringLiteral("html")},
            {QStringLiteral("html"), QStringLiteral("pdf")});

        QCOMPARE(formats.size(), 3);
        QCOMPARE(formats.at(0).id, QStringLiteral("html"));
        QVERIFY(formats.at(0).canRead);
        QVERIFY(formats.at(0).canWrite);
        QCOMPARE(formats.at(1).id, QStringLiteral("markdown"));
        QVERIFY(formats.at(1).canRead);
        QVERIFY(!formats.at(1).canWrite);
        QCOMPARE(formats.at(2).id, QStringLiteral("pdf"));
        QVERIFY(!formats.at(2).canRead);
        QVERIFY(formats.at(2).canWrite);
    }

    void discoversFormatsWithFakePandoc()
    {
        const PandocFormatService service(fakePandocPath());
        const auto result = service.discover();

        QVERIFY(result.success);
        QVERIFY(result.inputFormats.contains(QStringLiteral("markdown")));
        QVERIFY(result.outputFormats.contains(QStringLiteral("epub")));
        QVERIFY(result.outputFormats.contains(QStringLiteral("pdf")));
        QVERIFY(!result.formats.isEmpty());
    }

    void sortsPriorityFormatsFirst()
    {
        const auto sorted = sortFormatsByPriority(
            {
                QStringLiteral("zformat"),
                QStringLiteral("pdf"),
                QStringLiteral("html"),
                QStringLiteral("docx"),
                QStringLiteral("alpha"),
            },
            {
                QStringLiteral("html"),
                QStringLiteral("docx"),
                QStringLiteral("pdf"),
            });

        QCOMPARE(sorted, QStringList({
            QStringLiteral("html"),
            QStringLiteral("docx"),
            QStringLiteral("pdf"),
            QStringLiteral("alpha"),
            QStringLiteral("zformat"),
        }));
    }

    void fallsBackWhenPandocIsMissing()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const PandocFormatService service(tempDir.filePath(QStringLiteral("missing-pandoc.exe")));
        const auto result = service.discover();

        QVERIFY(!result.success);
        QCOMPARE(result.inputFormats, PandocFormatService::fallbackInputFormats());
        QCOMPARE(result.outputFormats, PandocFormatService::fallbackOutputFormats());
        QVERIFY(result.outputFormats.contains(QStringLiteral("html")));
    }

private:
    static QString fakePandocPath()
    {
        return QCoreApplication::applicationDirPath() + QStringLiteral("/fake_pandoc.exe");
    }
};

QTEST_MAIN(PandocFormatsTest)

#include "test_pandoc_formats.moc"
