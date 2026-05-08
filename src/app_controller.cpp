#include "app_controller.h"

#include "pandoc_formats.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QStringList>

namespace {

QString normalizeFormat(const QString &format)
{
    const auto normalized = Md2Any::normalizedOutputFormat(format);
    return normalized.isEmpty()
        ? QString::fromLatin1(Md2Any::DefaultOutputFormat)
        : normalized;
}

QString normalizeInputFormat(const QString &format)
{
    const auto normalized = Md2Any::normalizedInputFormat(format);
    return normalized.isEmpty()
        ? QString::fromLatin1(Md2Any::DefaultInputFormat)
        : normalized;
}

QStringList inputFormatPriority()
{
    return {
        QStringLiteral("markdown"),
        QStringLiteral("html"),
        QStringLiteral("docx"),
        QStringLiteral("latex"),
        QStringLiteral("typst"),
    };
}

QStringList outputFormatPriority()
{
    return {
        QStringLiteral("html"),
        QStringLiteral("docx"),
        QStringLiteral("pdf"),
        QStringLiteral("latex"),
        QStringLiteral("typst"),
        QStringLiteral("epub"),
    };
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
    connect(&m_conversionProcess, &Md2Any::ConversionProcess::finished, this, &AppController::finishProcess);
}

AppController::AppController(const QString &settingsPath, QObject *parent)
    : QObject(parent)
    , m_settingsStore(settingsPath)
{
    initializeFromSettings();
    connect(&m_conversionProcess, &Md2Any::ConversionProcess::finished, this, &AppController::finishProcess);
}

QString AppController::statusMessage() const
{
    return m_statusMessage;
}

QString AppController::formatStatusMessage() const
{
    return m_formatStatusMessage;
}

QString AppController::logText() const
{
    return m_logText;
}

QString AppController::pandocPath() const
{
    return m_runner.pandocPath();
}

QStringList AppController::outputFormats() const
{
    return m_outputFormats;
}

QStringList AppController::inputFormats() const
{
    return m_inputFormats;
}

QString AppController::defaultInputFormat() const
{
    return m_defaultInputFormat;
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

bool AppController::hasOutputFile() const
{
    return !m_lastOutputPath.isEmpty() && QFileInfo::exists(m_lastOutputPath);
}

bool AppController::valid() const
{
    return m_valid;
}

bool AppController::busy() const
{
    return m_busy;
}

void AppController::setPandocPath(const QString &pandocPath)
{
    const auto normalizedPath = pandocPath.trimmed().isEmpty()
        ? QString::fromLatin1(Md2Any::DefaultPandocPath)
        : pandocPath.trimmed();

    if (m_runner.pandocPath() == normalizedPath) {
        return;
    }

    m_runner = Md2Any::PandocRunner(normalizedPath, m_inputFormats, m_outputFormats);
    refreshFormats();
    emit pandocPathChanged();
}

void AppController::validateConversion(
    const QString &inputPath,
    const QString &outputPath,
    const QString &inputFormat,
    const QString &outputFormat,
    bool overwrite)
{
    if (m_busy) {
        return;
    }

    const auto task = makeTask(inputPath, outputPath, inputFormat, outputFormat, overwrite);
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
    const QString &inputFormat,
    const QString &outputFormat,
    bool overwrite)
{
    if (m_busy) {
        return;
    }

    const auto task = makeTask(inputPath, outputPath, inputFormat, outputFormat, overwrite);
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

    m_conversionProcess.start(program, arguments);
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
    m_conversionProcess.cancel();
}

void AppController::checkPandocPath(const QString &pandocPath)
{
    if (m_busy) {
        return;
    }

    const Md2Any::PandocRunner runner(pandocPath);
    const auto pandocStatus = runner.checkPandocAvailable();

    if (pandocStatus.success) {
        setStatusMessage(QStringLiteral("Pandoc 检测通过。"));
        const Md2Any::PandocFormatService formatService(runner.pandocPath());
        const auto formats = formatService.discover();
        setLogText(QStringLiteral("Pandoc：已找到\n路径：%1\n可写格式：%2 个")
            .arg(pandocStatus.command.join(QStringLiteral(" ")))
            .arg(formats.outputFormats.size()));
        return;
    }

    setValid(false);
    setStatusMessage(QStringLiteral("未找到 Pandoc。"));
    setLogText(QStringLiteral("Pandoc：%1").arg(pandocStatus.message));
}

void AppController::savePandocPath(const QString &pandocPath)
{
    if (m_busy) {
        return;
    }

    setPandocPath(pandocPath);
    m_settingsStore.save({
        m_runner.pandocPath(),
        m_lastInputDir,
        m_lastOutputDir,
        m_defaultInputFormat,
        m_defaultOutputFormat,
    });

    setStatusMessage(QStringLiteral("Pandoc 路径设置已保存。"));
    setLogText(QStringLiteral("已保存 Pandoc 路径：%1").arg(m_runner.pandocPath()));
}

void AppController::openOutputFile()
{
    if (m_lastOutputPath.isEmpty()) {
        setStatusMessage(QStringLiteral("还没有可打开的输出文件。"));
        return;
    }

    const QFileInfo outputInfo(m_lastOutputPath);
    if (!outputInfo.exists() || !outputInfo.isFile()) {
        setStatusMessage(QStringLiteral("输出文件不存在。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("输出文件不存在：%1").arg(outputInfo.absoluteFilePath()));
        emit hasOutputFileChanged();
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(outputInfo.absoluteFilePath()))) {
        setStatusMessage(QStringLiteral("无法打开输出文件。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("无法打开输出文件：%1").arg(outputInfo.absoluteFilePath()));
    }
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
    const auto extension = Md2Any::recommendedExtensionForFormat(format);
    QFileInfo info(trimmedPath);
    const auto suffix = info.suffix().toLower();

    if (suffix == extension) {
        return trimmedPath;
    }

    if (suffix.isEmpty()) {
        return trimmedPath + QStringLiteral(".") + extension;
    }

    const auto dir = info.dir().path();
    const auto completeBaseName = info.completeBaseName();
    return QDir(dir).filePath(completeBaseName + QStringLiteral(".") + extension);
}

Md2Any::ConversionTask AppController::makeTask(
    const QString &inputPath,
    const QString &outputPath,
    const QString &inputFormat,
    const QString &outputFormat,
    bool overwrite) const
{
    return {
        inputPath.trimmed(),
        normalizedOutputPath(outputPath, outputFormat),
        normalizeFormat(outputFormat),
        overwrite,
        {},
        normalizeInputFormat(inputFormat),
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
    const auto inputFormat = normalizeInputFormat(task.inputFormat);

    m_settingsStore.save({
        m_runner.pandocPath(),
        inputDir,
        outputDir,
        inputFormat,
        format,
    });

    setLastInputDir(inputDir);
    setLastOutputDir(outputDir);
    if (m_defaultInputFormat != inputFormat) {
        m_defaultInputFormat = inputFormat;
        emit defaultInputFormatChanged();
    }
    setDefaultOutputFormat(format);
}

void AppController::initializeFromSettings()
{
    const auto settings = m_settingsStore.load();
    m_runner = Md2Any::PandocRunner(settings.pandocPath);
    m_defaultInputFormat = settings.lastInputFormat;
    m_defaultOutputFormat = settings.lastOutputFormat;
    m_lastInputDir = settings.lastInputDir;
    m_lastOutputDir = settings.lastOutputDir;
    m_statusMessage = QStringLiteral("请选择文件并检查转换配置。");
    refreshFormats();
}

void AppController::refreshFormats()
{
    const Md2Any::PandocFormatService formatService(m_runner.pandocPath());
    const auto discovered = formatService.discover();
    const auto discoveredInputFormats = discovered.inputFormats.isEmpty()
        ? Md2Any::PandocFormatService::fallbackInputFormats()
        : discovered.inputFormats;
    const auto discoveredOutputFormats = discovered.outputFormats.isEmpty()
        ? Md2Any::PandocFormatService::fallbackOutputFormats()
        : discovered.outputFormats;
    const auto inputFormats = Md2Any::sortFormatsByPriority(discoveredInputFormats, inputFormatPriority());
    const auto outputFormats = Md2Any::sortFormatsByPriority(discoveredOutputFormats, outputFormatPriority());

    m_runner = Md2Any::PandocRunner(m_runner.pandocPath(), inputFormats, outputFormats);
    setFormatStatusMessage(discovered.success
        ? QStringLiteral("格式列表来自当前 Pandoc。")
        : QStringLiteral("未能读取 Pandoc 格式列表，正在使用保底格式。"));

    if (m_inputFormats != inputFormats) {
        m_inputFormats = inputFormats;
        emit inputFormatsChanged();
    }

    if (m_outputFormats != outputFormats) {
        m_outputFormats = outputFormats;
        emit outputFormatsChanged();
    }

    if (!m_inputFormats.contains(m_defaultInputFormat) && !m_inputFormats.isEmpty()) {
        m_defaultInputFormat = m_inputFormats.first();
        emit defaultInputFormatChanged();
    }

    if (!m_outputFormats.contains(m_defaultOutputFormat) && !m_outputFormats.isEmpty()) {
        setDefaultOutputFormat(m_outputFormats.first());
    }
}

void AppController::finishProcess(const Md2Any::ConversionProcessResult &result)
{
    const auto stdoutText = result.standardOutput;
    const auto stderrText = result.standardError;
    const auto success = result.exitStatus == QProcess::NormalExit && result.exitCode == 0 && !result.canceled;

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

    if (result.canceled) {
        setValid(false);
        setStatusMessage(QStringLiteral("转换已取消。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("转换已取消。"));
    } else if (success) {
        saveRecentSettings(m_runningTask);
        setLastOutputPath(QFileInfo(m_runningTask.outputPath).absoluteFilePath());
        setValid(true);
        setStatusMessage(QStringLiteral("转换成功。"));
        appendLogLine(QStringLiteral(""));
        appendLogLine(QStringLiteral("输出文件：%1").arg(QFileInfo(m_runningTask.outputPath).absoluteFilePath()));
    } else {
        setValid(false);
        setStatusMessage(QStringLiteral("转换失败。"));
        appendLogLine(QStringLiteral(""));
        if (!result.errorString.isEmpty()) {
            appendLogLine(QStringLiteral("进程错误：%1").arg(result.errorString));
        }
        appendLogLine(QStringLiteral("Pandoc 退出码：%1").arg(result.exitCode));
        appendLogLine(QStringLiteral("提示：当前输入/输出格式组合可能不受 Pandoc 支持，请查看上方错误输出。"));
    }

    setBusy(false);
}

void AppController::setStatusMessage(const QString &statusMessage)
{
    if (m_statusMessage == statusMessage) {
        return;
    }

    m_statusMessage = statusMessage;
    emit statusMessageChanged();
}

void AppController::setFormatStatusMessage(const QString &formatStatusMessage)
{
    if (m_formatStatusMessage == formatStatusMessage) {
        return;
    }

    m_formatStatusMessage = formatStatusMessage;
    emit formatStatusMessageChanged();
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

void AppController::setLastOutputPath(const QString &lastOutputPath)
{
    const auto normalizedPath = lastOutputPath.trimmed();
    if (m_lastOutputPath == normalizedPath) {
        return;
    }

    m_lastOutputPath = normalizedPath;
    emit hasOutputFileChanged();
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
