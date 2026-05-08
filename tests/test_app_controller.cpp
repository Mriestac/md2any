#include "app_controller.h"
#include "settings_store.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class AppControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void hasStableInitialState()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppController controller(tempDir.filePath(QStringLiteral("settings.ini")));

        QCOMPARE(controller.defaultOutputFormat(), QStringLiteral("html"));
        QCOMPARE(controller.defaultInputFormat(), QStringLiteral("markdown"));
        QCOMPARE(controller.pandocPath(), QStringLiteral("pandoc"));
        QVERIFY(controller.inputFormats().contains(QStringLiteral("markdown")));
        QVERIFY(controller.outputFormats().contains(QStringLiteral("html")));
        QVERIFY(controller.outputFormats().contains(QStringLiteral("docx")));
        QVERIFY(controller.outputFormats().contains(QStringLiteral("pdf")));
        QCOMPARE(controller.outputFormats().at(0), QStringLiteral("html"));
        QVERIFY(!controller.formatStatusMessage().isEmpty());
        QVERIFY(controller.lastInputDir().isEmpty());
        QVERIFY(controller.lastOutputDir().isEmpty());
        QVERIFY(!controller.hasOutputFile());
        QVERIFY(!controller.busy());
        QVERIFY(!controller.valid());
    }

    void normalizesOutputExtension()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppController controller(tempDir.filePath(QStringLiteral("settings.ini")));

        QCOMPARE(
            controller.normalizedOutputPath(tempDir.filePath(QStringLiteral("output")), QStringLiteral("html")),
            tempDir.filePath(QStringLiteral("output.html")));
        QCOMPARE(
            controller.normalizedOutputPath(tempDir.filePath(QStringLiteral("output.docx")), QStringLiteral("pdf")),
            tempDir.filePath(QStringLiteral("output.pdf")));
        QCOMPARE(
            controller.normalizedOutputPath(tempDir.filePath(QStringLiteral("output")), QStringLiteral("latex")),
            tempDir.filePath(QStringLiteral("output.tex")));
        QCOMPARE(
            controller.normalizedOutputPath(tempDir.filePath(QStringLiteral("output.txt")), QStringLiteral("typst")),
            tempDir.filePath(QStringLiteral("output.typ")));
    }

    void invalidInputShowsChineseError()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppController controller(tempDir.filePath(QStringLiteral("settings.ini")));
        controller.validateConversion(
            {},
            tempDir.filePath(QStringLiteral("output.html")),
            QStringLiteral("markdown"),
            QStringLiteral("html"),
            false);

        QVERIFY(controller.logText().contains(QStringLiteral("输入错误")));
        QVERIFY(!controller.valid());
    }

    void savesCustomPandocPath()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));
        const auto customPandocPath = fakePandocPath();

        AppController controller(settingsPath);
        controller.savePandocPath(customPandocPath);

        AppController reloadedController(settingsPath);
        QCOMPARE(reloadedController.pandocPath(), customPandocPath);
        QVERIFY(reloadedController.outputFormats().contains(QStringLiteral("pdf")));
    }

    void missingPandocPathShowsError()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppController controller(tempDir.filePath(QStringLiteral("settings.ini")));
        controller.checkPandocPath(tempDir.filePath(QStringLiteral("missing-pandoc.exe")));

        QVERIFY(!controller.valid());
        QCOMPARE(controller.statusMessage(), QStringLiteral("未找到 Pandoc。"));
        QVERIFY(controller.logText().contains(QStringLiteral("Pandoc")));
    }

    void missingPandocUsesFallbackFormatStatus()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));
        saveSettings(settingsPath, tempDir.filePath(QStringLiteral("missing-pandoc.exe")));

        AppController controller(settingsPath);

        QVERIFY(controller.formatStatusMessage().contains(QStringLiteral("保底格式")));
        QCOMPARE(controller.inputFormats(), QStringList({QStringLiteral("markdown")}));
        QCOMPARE(controller.outputFormats(), QStringList({
            QStringLiteral("html"),
            QStringLiteral("docx"),
            QStringLiteral("pdf"),
        }));
    }

    void unsupportedSavedFormatsFallBackToAvailableFormats()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));

        Md2Any::SettingsStore store(settingsPath);
        store.save({
            fakePandocPath(),
            {},
            {},
            QStringLiteral("missing-input-format"),
            QStringLiteral("missing-output-format"),
        });

        AppController controller(settingsPath);

        QVERIFY(controller.inputFormats().contains(controller.defaultInputFormat()));
        QVERIFY(controller.outputFormats().contains(controller.defaultOutputFormat()));
    }

    void openOutputFileWithoutResultShowsMessage()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppController controller(tempDir.filePath(QStringLiteral("settings.ini")));
        controller.openOutputFile();

        QVERIFY(!controller.hasOutputFile());
        QCOMPARE(controller.statusMessage(), QStringLiteral("还没有可打开的输出文件。"));
    }

    void fakePandocSuccessSavesRecentSettings()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));
        saveSettings(settingsPath, fakePandocPath());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = tempDir.filePath(QStringLiteral("output.html"));

        AppController controller(settingsPath);
        QSignalSpy busySpy(&controller, SIGNAL(busyChanged()));

        controller.startConversion(inputPath, outputPath, QStringLiteral("html"), QStringLiteral("html"), false);
        QVERIFY(waitUntilIdle(controller, busySpy));

        QVERIFY(controller.valid());
        QCOMPARE(controller.statusMessage(), QStringLiteral("转换成功。"));
        QVERIFY(QFile::exists(outputPath));
        QVERIFY(controller.hasOutputFile());
        QCOMPARE(controller.defaultInputFormat(), QStringLiteral("html"));
        QCOMPARE(controller.defaultOutputFormat(), QStringLiteral("html"));
        QCOMPARE(controller.lastInputDir(), tempDir.path());
        QCOMPARE(controller.lastOutputDir(), tempDir.path());

        AppController reloadedController(settingsPath);
        QCOMPARE(reloadedController.defaultInputFormat(), QStringLiteral("html"));
    }

    void fakePandocFailureWritesErrorLog()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));
        saveSettings(settingsPath, fakePandocPath());
        const auto inputPath = writeMarkdownFile(tempDir, QStringLiteral("input.md"));
        const auto outputPath = tempDir.filePath(QStringLiteral("fail.html"));

        AppController controller(settingsPath);
        QSignalSpy busySpy(&controller, SIGNAL(busyChanged()));

        controller.startConversion(inputPath, outputPath, QStringLiteral("markdown"), QStringLiteral("html"), false);
        QVERIFY(waitUntilIdle(controller, busySpy));

        QVERIFY(!controller.valid());
        QCOMPARE(controller.statusMessage(), QStringLiteral("转换失败。"));
        QVERIFY(controller.logText().contains(QStringLiteral("fake pandoc failure")));
        QVERIFY(controller.logText().contains(QStringLiteral("格式组合可能不受 Pandoc 支持")));
    }

private:
    static QString fakePandocPath()
    {
        return QCoreApplication::applicationDirPath() + QStringLiteral("/fake_pandoc.exe");
    }

    static void saveSettings(const QString &settingsPath, const QString &pandocPath)
    {
        Md2Any::SettingsStore store(settingsPath);
        store.save({
            pandocPath,
            {},
            {},
            QStringLiteral("markdown"),
            QStringLiteral("html"),
        });
    }

    static bool waitUntilIdle(AppController &controller, QSignalSpy &busySpy)
    {
        if (!controller.busy()) {
            return true;
        }

        for (int i = 0; i < 20 && controller.busy(); ++i) {
            busySpy.wait(250);
        }

        return !controller.busy();
    }

    static QString writeMarkdownFile(const QTemporaryDir &tempDir, const QString &fileName)
    {
        const auto path = tempDir.filePath(fileName);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qFatal("Failed to create test file: %s", qPrintable(path));
        }
        file.write("# Title\n");
        file.close();
        return path;
    }
};

QTEST_MAIN(AppControllerTest)

#include "test_app_controller.moc"
