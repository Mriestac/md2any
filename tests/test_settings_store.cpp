#include "settings_store.h"

#include <QDir>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Md2Any;

class SettingsStoreTest : public QObject
{
    Q_OBJECT

private slots:
    void missingSettingsFileReturnsDefaults()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const SettingsStore store(tempDir.filePath(QStringLiteral("missing.ini")));
        const auto settings = store.load();

        QCOMPARE(settings.pandocPath, QStringLiteral("pandoc"));
        QCOMPARE(settings.lastOutputFormat, QStringLiteral("html"));
        QVERIFY(settings.lastInputDir.isEmpty());
        QVERIFY(settings.lastOutputDir.isEmpty());
    }

    void savedSettingsCanBeReadBack()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const auto inputDir = tempDir.filePath(QStringLiteral("输入 path"));
        const auto outputDir = tempDir.filePath(QStringLiteral("output path"));
        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));

        const SettingsStore store(settingsPath);
        store.save({
            QStringLiteral("C:/Tools/Pandoc/pandoc.exe"),
            inputDir,
            outputDir,
            QStringLiteral("DOCX"),
        });

        const auto settings = store.load();

        QCOMPARE(settings.pandocPath, QStringLiteral("C:/Tools/Pandoc/pandoc.exe"));
        QCOMPARE(settings.lastInputDir, inputDir);
        QCOMPARE(settings.lastOutputDir, outputDir);
        QCOMPARE(settings.lastOutputFormat, QStringLiteral("docx"));
    }

    void emptyPandocPathAndInvalidFormatFallBackToDefaults()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const SettingsStore store(tempDir.filePath(QStringLiteral("settings.ini")));
        store.save({
            QStringLiteral("  "),
            {},
            {},
            QStringLiteral("epub"),
        });

        const auto settings = store.load();

        QCOMPARE(settings.pandocPath, QStringLiteral("pandoc"));
        QCOMPARE(settings.lastOutputFormat, QStringLiteral("html"));
    }

    void invalidSavedFormatLoadsAsDefault()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const auto settingsPath = tempDir.filePath(QStringLiteral("settings.ini"));
        QSettings rawSettings(settingsPath, QSettings::IniFormat);
        rawSettings.setValue(QStringLiteral("pandocPath"), QStringLiteral("pandoc"));
        rawSettings.setValue(QStringLiteral("lastOutputFormat"), QStringLiteral("epub"));
        rawSettings.sync();

        const SettingsStore store(settingsPath);
        const auto settings = store.load();

        QCOMPARE(settings.lastOutputFormat, QStringLiteral("html"));
    }
};

QTEST_MAIN(SettingsStoreTest)

#include "test_settings_store.moc"
