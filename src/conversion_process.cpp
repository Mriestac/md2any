#include "conversion_process.h"

#include <QFileInfo>

namespace Md2Any {

ConversionProcess::ConversionProcess(QObject *parent)
    : QObject(parent)
{
    connect(&m_process, &QProcess::finished, this, &ConversionProcess::handleFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &ConversionProcess::handleError);
}

bool ConversionProcess::isRunning() const
{
    return m_process.state() != QProcess::NotRunning;
}

void ConversionProcess::start(const QString &program, const QStringList &arguments)
{
    if (isRunning()) {
        return;
    }

    m_errorString.clear();
    m_canceled = false;
    m_finishedEmitted = false;

    const QFileInfo programInfo(program);
    if (program.trimmed().isEmpty() || (programInfo.isAbsolute() && !programInfo.exists())) {
        emitFinishedOnce({
            -1,
            QProcess::CrashExit,
            {},
            {},
            QStringLiteral("Program does not exist."),
            false,
        });
        return;
    }

    m_process.setProgram(program);
    m_process.setArguments(arguments);
    m_process.start();
}

void ConversionProcess::cancel()
{
    if (!isRunning()) {
        return;
    }

    m_canceled = true;
    m_process.kill();
}

void ConversionProcess::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emitFinishedOnce({
        exitCode,
        exitStatus,
        QString::fromLocal8Bit(m_process.readAllStandardOutput()).trimmed(),
        QString::fromLocal8Bit(m_process.readAllStandardError()).trimmed(),
        m_errorString,
        m_canceled,
    });
}

void ConversionProcess::handleError(QProcess::ProcessError error)
{
    m_errorString = m_process.errorString();

    if (error == QProcess::FailedToStart) {
        emitFinishedOnce({
            -1,
            QProcess::CrashExit,
            {},
            {},
            m_errorString,
            m_canceled,
        });
    }
}

void ConversionProcess::emitFinishedOnce(const ConversionProcessResult &result)
{
    if (m_finishedEmitted) {
        return;
    }

    m_finishedEmitted = true;
    emit finished(result);
}

} // namespace Md2Any
