#include "pandoc_runner.h"

#include <QFile>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Md2Any;

class PandocRunnerTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultsToPandocExecutable()
    {
        const PandocRunner runner;

        QCOMPARE(runner.pandocPath(), QStringLiteral("pandoc"));
        QCOMPARE(runner.inputFormats(), QStringList({QStringLiteral("markdown")}));
    }

    void detectsExistingCustomPandocPath()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto pandocPath = writeFile(tempDir, QStringLiteral("pandoc.exe"), "fake pandoc\n");

        const PandocRunner runner(pandocPath);
        const auto result = runner.checkPandocAvailable();

        QVERIFY(result.success);
        QCOMPARE(result.command, QStringList({pandocPath}));
    }

    void rejectsMissingCustomPandocPath()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto pandocPath = tempDir.filePath(QStringLiteral("missing-pandoc.exe"));

        const PandocRunner runner(pandocPath);
        const auto result = runner.checkPandocAvailable();

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Pandoc executable does not exist."));
    }

    void rejectsEmptyInputPath()
    {
        const PandocRunner runner;
        const auto result = runner.validate({
            {},
            QStringLiteral("output.html"),
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Input file is required."));
    }

    void rejectsMissingInputFile()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const PandocRunner runner;
        const auto result = runner.validate({
            tempDir.filePath(QStringLiteral("missing.md")),
            tempDir.filePath(QStringLiteral("output.html")),
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Input file does not exist."));
    }

    void rejectsEmptyOutputPath()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));

        const PandocRunner runner;
        const auto result = runner.validate({
            inputPath,
            {},
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Output file is required."));
    }

    void rejectsUnsupportedOutputFormat()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));

        const PandocRunner runner;
        const auto result = runner.validate({
            inputPath,
            tempDir.filePath(QStringLiteral("output.epub")),
            QStringLiteral("epub"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Unsupported output format."));
    }

    void rejectsUnsupportedInputFormat()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));

        const PandocRunner runner;
        const auto result = runner.validate({
            inputPath,
            tempDir.filePath(QStringLiteral("output.html")),
            QStringLiteral("html"),
            false,
            {},
            QStringLiteral("docx"),
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Unsupported input format."));
    }

    void rejectsMissingOutputDirectory()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));

        const PandocRunner runner;
        const auto result = runner.validate({
            inputPath,
            tempDir.filePath(QStringLiteral("missing-dir/output.html")),
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Output directory does not exist."));
    }

    void rejectsExistingOutputWhenOverwriteIsFalse()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = writeFile(tempDir, QStringLiteral("output.html"), "<html></html>\n");

        const PandocRunner runner;
        const auto result = runner.validate({
            inputPath,
            outputPath,
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Output file already exists."));
    }

    void acceptsExistingOutputWhenOverwriteIsTrue()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = writeFile(tempDir, QStringLiteral("output.html"), "<html></html>\n");

        const PandocRunner runner;
        const auto result = runner.validate({
            inputPath,
            outputPath,
            QStringLiteral("HTML"),
            true,
            {},
        });

        QVERIFY(result.success);
        QCOMPARE(result.command, QStringList({
            QStringLiteral("pandoc"),
            QStringLiteral("--standalone"),
            QStringLiteral("--mathjax"),
            QStringLiteral("-f"),
            QStringLiteral("markdown"),
            QStringLiteral("-t"),
            QStringLiteral("html"),
            inputPath,
            QStringLiteral("-o"),
            outputPath,
        }));
    }

    void preservesSpacePathsInCommand()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input file.md"));
        const auto outputPath = tempDir.filePath(QStringLiteral("output file.docx"));

        const PandocRunner runner(QStringLiteral("C:/Pandoc/pandoc.exe"));
        const auto command = runner.buildCommand({
            inputPath,
            outputPath,
            QStringLiteral("docx"),
            false,
            {},
        });

        QCOMPARE(command, QStringList({
            QStringLiteral("C:/Pandoc/pandoc.exe"),
            QStringLiteral("-f"),
            QStringLiteral("markdown"),
            QStringLiteral("-t"),
            QStringLiteral("docx"),
            inputPath,
            QStringLiteral("-o"),
            outputPath,
        }));
    }

    void keepsExtraArgumentsBeforeInputPath()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = tempDir.filePath(QStringLiteral("output.html"));

        const PandocRunner runner;
        const auto command = runner.buildCommand({
            inputPath,
            outputPath,
            QStringLiteral("html"),
            false,
            {QStringLiteral("--standalone"), QStringLiteral("--toc")},
        });

        QCOMPARE(command, QStringList({
            QStringLiteral("pandoc"),
            QStringLiteral("--standalone"),
            QStringLiteral("--toc"),
            QStringLiteral("--mathjax"),
            QStringLiteral("-f"),
            QStringLiteral("markdown"),
            QStringLiteral("-t"),
            QStringLiteral("html"),
            inputPath,
            QStringLiteral("-o"),
            outputPath,
        }));
    }

    void runReturnsValidationFailureWithoutStartingPandoc()
    {
        const PandocRunner runner(QStringLiteral("C:/missing/pandoc.exe"));
        const auto result = runner.run({
            {},
            QStringLiteral("output.html"),
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.message, QStringLiteral("Pandoc executable does not exist."));
    }

    void runCreatesOutputWithFakePandoc()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = tempDir.filePath(QStringLiteral("output.html"));

        const PandocRunner runner(fakePandocPath());
        const auto result = runner.run({
            inputPath,
            outputPath,
            QStringLiteral("html"),
            false,
            {},
        });

        QVERIFY(result.success);
        QCOMPARE(result.exitCode, 0);
        QVERIFY(QFile::exists(outputPath));
        QCOMPARE(result.outputPath, outputPath);
    }

    void runCapturesFakePandocFailure()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = tempDir.filePath(QStringLiteral("output.html"));

        const PandocRunner runner(fakePandocPath());
        const auto result = runner.run({
            inputPath,
            outputPath,
            QStringLiteral("html"),
            false,
            {QStringLiteral("--fail")},
        });

        QVERIFY(!result.success);
        QCOMPARE(result.exitCode, 7);
        QVERIFY(result.standardError.contains(QStringLiteral("fake pandoc failure")));
        QCOMPARE(result.message, QStringLiteral("Pandoc conversion failed."));
    }

private:
    static QString fakePandocPath()
    {
        return QCoreApplication::applicationDirPath() + QStringLiteral("/fake_pandoc.exe");
    }

    static QString writeMarkdownFile(const QTemporaryDir &tempDir, const QString &fileName)
    {
        return writeFile(tempDir, fileName, "# Title\n");
    }

    static QString writeFile(const QTemporaryDir &tempDir, const QString &fileName, const char *content)
    {
        const auto path = tempDir.filePath(fileName);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qFatal("Failed to create test file: %s", qPrintable(path));
        }
        file.write(content);
        file.close();
        return path;
    }
};

QTEST_MAIN(PandocRunnerTest)

#include "test_pandoc_runner.moc"
