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
        QVERIFY(controller.lastInputDir().isEmpty());
        QVERIFY(controller.lastOutputDir().isEmpty());
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
    }

    void invalidInputShowsChineseError()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppController controller(tempDir.filePath(QStringLiteral("settings.ini")));
        controller.validateConversion({}, tempDir.filePath(QStringLiteral("output.html")), QStringLiteral("html"), false);

        QVERIFY(controller.logText().contains(QStringLiteral("输入错误")));
        QVERIFY(!controller.valid());
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

        controller.startConversion(inputPath, outputPath, QStringLiteral("html"), false);
        QVERIFY(waitUntilIdle(controller, busySpy));

        QVERIFY(controller.valid());
        QCOMPARE(controller.statusMessage(), QStringLiteral("转换成功。"));
        QVERIFY(QFile::exists(outputPath));
        QCOMPARE(controller.defaultOutputFormat(), QStringLiteral("html"));
        QCOMPARE(controller.lastInputDir(), tempDir.path());
        QCOMPARE(controller.lastOutputDir(), tempDir.path());
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

        controller.startConversion(inputPath, outputPath, QStringLiteral("html"), false);
        QVERIFY(waitUntilIdle(controller, busySpy));

        QVERIFY(!controller.valid());
        QCOMPARE(controller.statusMessage(), QStringLiteral("转换失败。"));
        QVERIFY(controller.logText().contains(QStringLiteral("fake pandoc failure")));
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
