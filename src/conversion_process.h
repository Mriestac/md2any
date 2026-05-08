#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

namespace Md2Any {

struct ConversionProcessResult
{
    int exitCode = -1;
    QProcess::ExitStatus exitStatus = QProcess::CrashExit;
    QString standardOutput;
    QString standardError;
    QString errorString;
    bool canceled = false;
};

class ConversionProcess : public QObject
{
    Q_OBJECT

public:
    explicit ConversionProcess(QObject *parent = nullptr);

    bool isRunning() const;
    void start(const QString &program, const QStringList &arguments);
    void cancel();

signals:
    void finished(const Md2Any::ConversionProcessResult &result);

private:
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleError(QProcess::ProcessError error);
    void emitFinishedOnce(const ConversionProcessResult &result);

    QProcess m_process;
    QString m_errorString;
    bool m_canceled = false;
    bool m_finishedEmitted = false;
};

} // namespace Md2Any
