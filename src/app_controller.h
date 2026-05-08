#pragma once

#include "conversion_process.h"
#include "conversion_task.h"
#include "pandoc_runner.h"
#include "settings_store.h"

#include <QObject>
#include <QString>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString formatStatusMessage READ formatStatusMessage NOTIFY formatStatusMessageChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)
    Q_PROPERTY(QString pandocPath READ pandocPath WRITE setPandocPath NOTIFY pandocPathChanged)
    Q_PROPERTY(QStringList inputFormats READ inputFormats NOTIFY inputFormatsChanged)
    Q_PROPERTY(QStringList outputFormats READ outputFormats NOTIFY outputFormatsChanged)
    Q_PROPERTY(QString defaultInputFormat READ defaultInputFormat NOTIFY defaultInputFormatChanged)
    Q_PROPERTY(QString defaultOutputFormat READ defaultOutputFormat NOTIFY defaultOutputFormatChanged)
    Q_PROPERTY(QString lastInputDir READ lastInputDir NOTIFY lastInputDirChanged)
    Q_PROPERTY(QString lastOutputDir READ lastOutputDir NOTIFY lastOutputDirChanged)
    Q_PROPERTY(bool hasOutputFile READ hasOutputFile NOTIFY hasOutputFileChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(const QString &settingsPath, QObject *parent = nullptr);

    QString statusMessage() const;
    QString formatStatusMessage() const;
    QString logText() const;
    QString pandocPath() const;
    QStringList inputFormats() const;
    QStringList outputFormats() const;
    QString defaultInputFormat() const;
    QString defaultOutputFormat() const;
    QString lastInputDir() const;
    QString lastOutputDir() const;
    bool hasOutputFile() const;
    bool valid() const;
    bool busy() const;

    void setPandocPath(const QString &pandocPath);

    Q_INVOKABLE void validateConversion(
        const QString &inputPath,
        const QString &outputPath,
        const QString &inputFormat,
        const QString &outputFormat,
        bool overwrite);
    Q_INVOKABLE void startConversion(
        const QString &inputPath,
        const QString &outputPath,
        const QString &inputFormat,
        const QString &outputFormat,
        bool overwrite);
    Q_INVOKABLE void cancelConversion();
    Q_INVOKABLE void checkPandocPath(const QString &pandocPath);
    Q_INVOKABLE void savePandocPath(const QString &pandocPath);
    Q_INVOKABLE void openOutputFile();
    Q_INVOKABLE void openOutputDirectory();
    Q_INVOKABLE QString normalizedOutputPath(const QString &outputPath, const QString &outputFormat) const;

signals:
    void statusMessageChanged();
    void formatStatusMessageChanged();
    void logTextChanged();
    void pandocPathChanged();
    void inputFormatsChanged();
    void outputFormatsChanged();
    void defaultInputFormatChanged();
    void defaultOutputFormatChanged();
    void lastInputDirChanged();
    void lastOutputDirChanged();
    void hasOutputFileChanged();
    void validChanged();
    void busyChanged();

private:
    Md2Any::ConversionTask makeTask(
        const QString &inputPath,
        const QString &outputPath,
        const QString &inputFormat,
        const QString &outputFormat,
        bool overwrite) const;
    QString commandPreview(const QStringList &command) const;
    void appendLogLine(const QString &line);
    void saveRecentSettings(const Md2Any::ConversionTask &task);
    void initializeFromSettings();
    void refreshFormats();
    void finishProcess(const Md2Any::ConversionProcessResult &result);

    void setStatusMessage(const QString &statusMessage);
    void setFormatStatusMessage(const QString &formatStatusMessage);
    void setLogText(const QString &logText);
    void setDefaultOutputFormat(const QString &defaultOutputFormat);
    void setLastInputDir(const QString &lastInputDir);
    void setLastOutputDir(const QString &lastOutputDir);
    void setLastOutputPath(const QString &lastOutputPath);
    void setValid(bool valid);
    void setBusy(bool busy);

    Md2Any::SettingsStore m_settingsStore;
    Md2Any::PandocRunner m_runner;
    Md2Any::ConversionTask m_runningTask;
    Md2Any::ConversionProcess m_conversionProcess;
    QString m_lastOutputPath;
    QStringList m_inputFormats;
    QStringList m_outputFormats;
    QString m_statusMessage;
    QString m_formatStatusMessage;
    QString m_logText;
    QString m_defaultInputFormat;
    QString m_defaultOutputFormat;
    QString m_lastInputDir;
    QString m_lastOutputDir;
    bool m_valid = false;
    bool m_busy = false;
    bool m_cancelRequested = false;
};
