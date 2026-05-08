#include "app_controller.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QStringList>

namespace {

QString normalizeFormat(const QString &format)
{
    const auto normalized = Md2Any::normalizedOutputFormat(format);
    return Md2Any::isSupportedOutputFormat(normalized)
        ? normalized
        : QString::fromLatin1(Md2Any::DefaultOutputFormat);
}

QString quotedForPreview(const QString &value)
{
    if (!value.contains(QLatin1Char(' '))) {
        return value;
    }

    auto escaped = value;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    return QStringLiteral("\"%1\"").arg(escaped);
}

} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    initializeFromSettings();
    connect(&m_process, &QProcess::finished, this, &AppController::finishProcess);
    connect(&m_process, &QProcess::errorOccurred, this, &AppController::failProcess);
}

AppController::AppController(const QString &settingsPath, QObject *parent)
    : QObject(parent)
    , m_settingsStore(settingsPath)
{
    initializeFromSettings();
    connect(&m_process, &QProcess::finished, this, &AppController::finishProcess);
    connect(&m_process, &QProcess::errorOccurred, this, &AppController::failProcess);
}

QString AppController::statusMessage() const
{
    return m_statusMessage;
}

QString AppController::logText() const
{
    return m_logText;
}

QString AppController::defaultOutputFormat() const
{
    return m_defaultOutputFormat;
}

QString AppController::lastInputDir() const
{
    return m_lastInputDir;
}

QString AppController::lastOutputDir() const
{
    return m_lastOutputDir;
}

bool AppController::valid() const
{
    return m_valid;
}

bool AppController::busy() const
{
    return m_busy;
}

void AppController::validateConversion(
    const QString &inputPath,
    const QString &outputPath,
    const QString &outputFormat,
    bool overwrite)
{
    if (m_busy) {
        return;
    }

    const auto task = makeTask(inputPath, outputPath, outputFormat, overwrite);
    const auto validation = m_runner.validate(task);
    const auto pandocStatus = m_runner.checkPandocAvailable();
    setValid(validation.success && pandocStatus.success);

    QStringList logLines;

    if (!validation.success) {
        setStatusMessage(QStringLiteral("输入配置有误。"));
        logLines << QStringLiteral("输入错误：%1").arg(validation.message);
        logLines << QStringLiteral("Pandoc：%1").arg(pandocStatus.success ? QStringLiteral("已找到") : pandocStatus.message);
        setLogText(logLines.join(QStringLiteral("\n")));
        return;
    }

    logLines << QStringLiteral("输入配置有效。");
    logLines << QStringLiteral("Pandoc：%1").arg(pandocStatus.success ? QStringLiteral("已找到") : pandocStatus.message);
    logLines << QStringLiteral("命令预览：");
    logLines << commandPreview(validation.command);

    if (!pandocStatus.success) {
        setStatusMessage(QStringLiteral("未找到 Pandoc。"));
        setLogText(logLines.join(QStringLiteral("\n")));
        return;
    }

    setStatusMessage(QStringLiteral("可以开始转换。"));
    setLogText(logLines.join(QStringLiteral("\n")));
}

void AppController::startConversion(
    const QString &inputPath,
    const QString &outputPath,
    const QString &outputFormat,
    bool overwrite)
{
    if (m_busy) {
        return;
    }

    const auto task = makeTask(inputPath, outputPath, outputFormat, overwrite);
    const auto validation = m_runner.validate(task);
    const auto pandocStatus = m_runner.checkPandocAvailable();

    if (!validation.success) {
        setValid(false);
        setStatusMessage(QStringLiteral("输入配置有误。"));
        setLogText(QStringLiteral("输入错误：%1").arg(validation.message));
        return;
    }

    if (!pandocStatus.success) {
        setValid(false);
        setStatusMessage(QStringLiteral("未找到 Pandoc。"));
        setLogText(QStringLiteral("Pandoc：%1").arg(pandocStatus.message));
        return;
    }

    m_runningTask = task;
    m_cancelRequested = false;
    const auto program = pandocStatus.command.isEmpty() ? m_runner.pandocPath() : pandocStatus.command.first();
    const auto arguments = m_runner.buildArguments(task);

    setBusy(true);
    setValid(false);
    setStatusMessage(QStringLiteral("正在转换..."));
    setLogText(QStringLiteral("正在调用 Pandoc，请稍候...\n\n命令：\n%1").arg(commandPreview(QStringList({program}) + arguments)));

    m_process.setProgram(program);
    m_process.setArguments(arguments);
    m_process.start();
}

void AppController::cancelConversion()
{
    if (!m_busy) {
        return;
    }

    m_cancelRequested = true;
    setStatusMessage(QStringLiteral("正在取消..."));
    appendLogLine(QStringLiteral(""));
    appendLogLine(QStringLiteral("正在取消转换。"));
    m_process.kill();
}

void AppController::openOutputDirectory()
{
    if (m_lastOutputPath.isEmpty()) {
        setStatusMessage(QStringLiteral("还没有可打开的输出目录。"));
        return;
    }

    const QFileInfo outputInfo(m_lastOutputPath);
    const auto directoryUrl = QUrl::fromLocalFile(outputInfo.absolutePath());

    if (!QDesktopServices::openUrl(directoryUrl)) {
        setStatusMessage(QStringLiteral("无法打开输出目录。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("无法打开输出目录：%1").arg(outputInfo.absolutePath()));
    }
}

QString AppController::normalizedOutputPath(const QString &outputPath, const QString &outputFormat) const
{
    const auto trimmedPath = outputPath.trimmed();
    if (trimmedPath.isEmpty()) {
        return {};
    }

    const auto format = normalizeFormat(outputFormat);
    QFileInfo info(trimmedPath);
    const auto suffix = info.suffix().toLower();

    if (suffix == format) {
        return trimmedPath;
    }

    if (suffix.isEmpty()) {
        return trimmedPath + QStringLiteral(".") + format;
    }

    const auto dir = info.dir().path();
    const auto completeBaseName = info.completeBaseName();
    return QDir(dir).filePath(completeBaseName + QStringLiteral(".") + format);
}

Md2Any::ConversionTask AppController::makeTask(
    const QString &inputPath,
    const QString &outputPath,
    const QString &outputFormat,
    bool overwrite) const
{
    return {
        inputPath.trimmed(),
        normalizedOutputPath(outputPath, outputFormat),
        normalizeFormat(outputFormat),
        overwrite,
        {},
    };
}

QString AppController::commandPreview(const QStringList &command) const
{
    QStringList preview;
    for (const auto &part : command) {
        preview << quotedForPreview(part);
    }
    return preview.join(QStringLiteral(" "));
}

void AppController::appendLogLine(const QString &line)
{
    setLogText(m_logText.isEmpty() ? line : m_logText + QStringLiteral("\n") + line);
}

void AppController::saveRecentSettings(const Md2Any::ConversionTask &task)
{
    const QFileInfo inputInfo(task.inputPath);
    const QFileInfo outputInfo(task.outputPath);
    const auto inputDir = inputInfo.absolutePath();
    const auto outputDir = outputInfo.absolutePath();
    const auto format = normalizeFormat(task.outputFormat);

    m_settingsStore.save({
        m_runner.pandocPath(),
        inputDir,
        outputDir,
        format,
    });

    setLastInputDir(inputDir);
    setLastOutputDir(outputDir);
    setDefaultOutputFormat(format);
}

void AppController::initializeFromSettings()
{
    const auto settings = m_settingsStore.load();
    m_runner = Md2Any::PandocRunner(settings.pandocPath);
    m_defaultOutputFormat = settings.lastOutputFormat;
    m_lastInputDir = settings.lastInputDir;
    m_lastOutputDir = settings.lastOutputDir;
    m_statusMessage = QStringLiteral("请选择文件并检查转换配置。");
}

void AppController::finishProcess(int exitCode, QProcess::ExitStatus exitStatus)
{
    const auto stdoutText = QString::fromLocal8Bit(m_process.readAllStandardOutput()).trimmed();
    const auto stderrText = QString::fromLocal8Bit(m_process.readAllStandardError()).trimmed();
    const auto success = exitStatus == QProcess::NormalExit && exitCode == 0 && !m_cancelRequested;

    if (!stdoutText.isEmpty()) {
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("标准输出："));
        appendLogLine(stdoutText);
    }

    if (!stderrText.isEmpty()) {
        appendLogLine(QStringLiteral(""));
        appendLogLine(success ? QStringLiteral("警告输出：") : QStringLiteral("错误输出："));
        appendLogLine(stderrText);
    }

    if (m_cancelRequested) {
        setValid(false);
        setStatusMessage(QStringLiteral("转换已取消。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("转换已取消。"));
    } else if (success) {
        saveRecentSettings(m_runningTask);
        m_lastOutputPath = QFileInfo(m_runningTask.outputPath).absoluteFilePath();
        setValid(true);
        setStatusMessage(QStringLiteral("转换成功。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("输出文件：%1").arg(QFileInfo(m_runningTask.outputPath).absoluteFilePath()));
    } else {
        setValid(false);
        setStatusMessage(QStringLiteral("转换失败。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("Pandoc 退出码：%1").arg(exitCode));
    }

    setBusy(false);
}

void AppController::failProcess(QProcess::ProcessError error)
{
    if (error == QProcess::Crashed && m_cancelRequested) {
        return;
    }

    setValid(false);
    setBusy(false);
    setStatusMessage(QStringLiteral("转换失败。"));
    appendLogLine(QStringLiteral(""));
    appendLogLine(QStringLiteral("进程错误：%1").arg(m_process.errorString()));
}

void AppController::setStatusMessage(const QString &statusMessage)
{
    if (m_statusMessage == statusMessage) {
        return;
    }

    m_statusMessage = statusMessage;
    emit statusMessageChanged();
}

void AppController::setLogText(const QString &logText)
{
    if (m_logText == logText) {
        return;
    }

    m_logText = logText;
    emit logTextChanged();
}

void AppController::setDefaultOutputFormat(const QString &defaultOutputFormat)
{
    if (m_defaultOutputFormat == defaultOutputFormat) {
        return;
    }

    m_defaultOutputFormat = defaultOutputFormat;
    emit defaultOutputFormatChanged();
}

void AppController::setLastInputDir(const QString &lastInputDir)
{
    if (m_lastInputDir == lastInputDir) {
        return;
    }

    m_lastInputDir = lastInputDir;
    emit lastInputDirChanged();
}

void AppController::setLastOutputDir(const QString &lastOutputDir)
{
    if (m_lastOutputDir == lastOutputDir) {
        return;
    }

    m_lastOutputDir = lastOutputDir;
    emit lastOutputDirChanged();
}

void AppController::setValid(bool valid)
{
    if (m_valid == valid) {
        return;
    }

    m_valid = valid;
    emit validChanged();
}

void AppController::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}
