#include "pandoc_runner.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <utility>

namespace Md2Any {

namespace {

ConversionResult failedResult(const QString &message)
{
    return {
        false,
        {},
        -1,
        {},
        message,
        {},
        message,
    };
}

} // namespace

PandocRunner::PandocRunner(QString pandocPath, QStringList inputFormats, QStringList outputFormats)
    : m_pandocPath(std::move(pandocPath))
    , m_inputFormats(std::move(inputFormats))
    , m_outputFormats(std::move(outputFormats))
{
    m_pandocPath = m_pandocPath.trimmed();

    if (m_pandocPath.isEmpty()) {
        m_pandocPath = QString::fromLatin1(DefaultPandocPath);
    }

    if (m_inputFormats.isEmpty()) {
        m_inputFormats = supportedInputFormats();
    }

    if (m_outputFormats.isEmpty()) {
        m_outputFormats = supportedOutputFormats();
    }
}

QString PandocRunner::pandocPath() const
{
    return m_pandocPath;
}

QStringList PandocRunner::inputFormats() const
{
    return m_inputFormats;
}

QStringList PandocRunner::outputFormats() const
{
    return m_outputFormats;
}

ConversionResult PandocRunner::checkPandocAvailable() const
{
    const QFileInfo pandocInfo(m_pandocPath);

    if (pandocInfo.isAbsolute()) {
        if (pandocInfo.exists() && pandocInfo.isFile()) {
            return {
                true,
                {m_pandocPath},
                0,
                {},
                {},
                {},
                QStringLiteral("Pandoc executable found."),
            };
        }

        return failedResult(QStringLiteral("Pandoc executable does not exist."));
    }

    const auto executable = QStandardPaths::findExecutable(m_pandocPath);

    if (!executable.isEmpty()) {
        return {
            true,
            {executable},
            0,
            {},
            {},
            {},
            QStringLiteral("Pandoc executable found."),
        };
    }

    return failedResult(QStringLiteral("Pandoc executable was not found in PATH."));
}

ConversionResult PandocRunner::validate(const ConversionTask &task) const
{
    if (task.inputPath.trimmed().isEmpty()) {
        return failedResult(QStringLiteral("Input file is required."));
    }

    const QFileInfo inputInfo(task.inputPath);

    if (!inputInfo.exists() || !inputInfo.isFile()) {
        return failedResult(QStringLiteral("Input file does not exist."));
    }

    if (task.outputPath.trimmed().isEmpty()) {
        return failedResult(QStringLiteral("Output file is required."));
    }

    const auto inputFormat = normalizedInputFormat(task.inputFormat);
    const auto format = normalizedOutputFormat(task.outputFormat);

    if (!m_inputFormats.contains(inputFormat)) {
        return failedResult(QStringLiteral("Unsupported input format."));
    }

    if (!m_outputFormats.contains(format)) {
        return failedResult(QStringLiteral("Unsupported output format."));
    }

    const QFileInfo outputInfo(task.outputPath);
    const auto outputDir = outputInfo.dir();

    if (!outputDir.exists()) {
        return failedResult(QStringLiteral("Output directory does not exist."));
    }

    if (outputInfo.exists() && !task.overwrite) {
        return failedResult(QStringLiteral("Output file already exists."));
    }

    return {
        true,
        buildCommand(task),
        0,
        {},
        {},
        outputInfo.absoluteFilePath(),
        QStringLiteral("Validation passed."),
    };
}

ConversionResult PandocRunner::run(const ConversionTask &task, int timeoutMs) const
{
    const auto pandocStatus = checkPandocAvailable();
    if (!pandocStatus.success) {
        return pandocStatus;
    }

    const auto validation = validate(task);
    if (!validation.success) {
        return validation;
    }

    const auto program = pandocStatus.command.isEmpty() ? m_pandocPath : pandocStatus.command.first();
    const auto arguments = buildArguments(task);

    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.start();

    if (!process.waitForStarted()) {
        return {
            false,
            buildCommand(task),
            -1,
            {},
            process.errorString(),
            {},
            QStringLiteral("Failed to start Pandoc."),
        };
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        return {
            false,
            buildCommand(task),
            -1,
            QString::fromLocal8Bit(process.readAllStandardOutput()),
            QStringLiteral("Pandoc timed out."),
            {},
            QStringLiteral("Pandoc timed out."),
        };
    }

    const auto exitCode = process.exitCode();
    const auto stdoutText = QString::fromLocal8Bit(process.readAllStandardOutput());
    const auto stderrText = QString::fromLocal8Bit(process.readAllStandardError());
    const auto success = process.exitStatus() == QProcess::NormalExit && exitCode == 0;

    return {
        success,
        QStringList({program}) + arguments,
        exitCode,
        stdoutText,
        stderrText,
        QFileInfo(task.outputPath).absoluteFilePath(),
        success ? QStringLiteral("Conversion succeeded.") : QStringLiteral("Pandoc conversion failed."),
    };
}

QStringList PandocRunner::buildArguments(const ConversionTask &task) const
{
    QStringList args;
    args << task.extraArgs;
    if (normalizedOutputFormat(task.outputFormat) == QStringLiteral("html")) {
        if (!args.contains(QStringLiteral("--standalone")) && !args.contains(QStringLiteral("-s"))) {
            args << QStringLiteral("--standalone");
        }
        args << QStringLiteral("--mathjax");
    }
    args << QStringLiteral("-f");
    args << normalizedInputFormat(task.inputFormat);
    args << QStringLiteral("-t");
    args << normalizedOutputFormat(task.outputFormat);
    args << task.inputPath;
    args << QStringLiteral("-o");
    args << task.outputPath;
    return args;
}

QStringList PandocRunner::buildCommand(const ConversionTask &task) const
{
    QStringList command;
    command << m_pandocPath;
    command << buildArguments(task);
    return command;
}

QString normalizedInputFormat(const QString &format)
{
    const auto normalized = format.trimmed().toLower();
    return normalized.isEmpty() ? QString::fromLatin1(DefaultInputFormat) : normalized;
}

QString normalizedOutputFormat(const QString &format)
{
    return format.trimmed().toLower();
}

} // namespace Md2Any
