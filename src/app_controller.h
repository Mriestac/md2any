#pragma once

#include "conversion_task.h"
#include "pandoc_runner.h"
#include "settings_store.h"

#include <QObject>
#include <QProcess>
#include <QString>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)
    Q_PROPERTY(QString defaultOutputFormat READ defaultOutputFormat NOTIFY defaultOutputFormatChanged)
    Q_PROPERTY(QString lastInputDir READ lastInputDir NOTIFY lastInputDirChanged)
    Q_PROPERTY(QString lastOutputDir READ lastOutputDir NOTIFY lastOutputDirChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(const QString &settingsPath, QObject *parent = nullptr);

    QString statusMessage() const;
    QString logText() const;
    QString defaultOutputFormat() const;
    QString lastInputDir() const;
    QString lastOutputDir() const;
    bool valid() const;
    bool busy() const;

    Q_INVOKABLE void validateConversion(
        const QString &inputPath,
        const QString &outputPath,
        const QString &outputFormat,
        bool overwrite);
    Q_INVOKABLE void startConversion(
        const QString &inputPath,
        const QString &outputPath,
        const QString &outputFormat,
        bool overwrite);
    Q_INVOKABLE void cancelConversion();
    Q_INVOKABLE void openOutputDirectory();
    Q_INVOKABLE QString normalizedOutputPath(const QString &outputPath, const QString &outputFormat) const;

signals:
    void statusMessageChanged();
    void logTextChanged();
    void defaultOutputFormatChanged();
    void lastInputDirChanged();
    void lastOutputDirChanged();
    void validChanged();
    void busyChanged();

private:
    Md2Any::ConversionTask makeTask(
        const QString &inputPath,
        const QString &outputPath,
        const QString &outputFormat,
        bool overwrite) const;
    QString commandPreview(const QStringList &command) const;
    void appendLogLine(const QString &line);
    void saveRecentSettings(const Md2Any::ConversionTask &task);
    void initializeFromSettings();
    void finishProcess(int exitCode, QProcess::ExitStatus exitStatus);
    void failProcess(QProcess::ProcessError error);

    void setStatusMessage(const QString &statusMessage);
    void setLogText(const QString &logText);
    void setDefaultOutputFormat(const QString &defaultOutputFormat);
    void setLastInputDir(const QString &lastInputDir);
    void setLastOutputDir(const QString &lastOutputDir);
    void setValid(bool valid);
    void setBusy(bool busy);

    Md2Any::SettingsStore m_settingsStore;
    Md2Any::PandocRunner m_runner;
    Md2Any::ConversionTask m_runningTask;
    QProcess m_process;
    QString m_lastOutputPath;
    QString m_statusMessage;
    QString m_logText;
    QString m_defaultOutputFormat;
    QString m_lastInputDir;
    QString m_lastOutputDir;
    bool m_valid = false;
    bool m_busy = false;
    bool m_cancelRequested = false;
};
