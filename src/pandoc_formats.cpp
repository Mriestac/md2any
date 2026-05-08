#include "pandoc_formats.h"

#include "conversion_task.h"

#include <QFileInfo>
#include <QProcess>
#include <QSet>
#include <QStandardPaths>
#include <algorithm>
#include <utility>

namespace Md2Any {

namespace {

QString normalizeFormatId(const QString &value)
{
    return value.trimmed().toLower();
}

QStringList parseFormatLines(const QString &text)
{
    QStringList formats;
    const auto lines = text.split(QLatin1Char('\n'));

    for (const auto &line : lines) {
        const auto format = normalizeFormatId(line);
        if (!format.isEmpty() && !formats.contains(format)) {
            formats << format;
        }
    }

    formats.sort();
    return formats;
}

QStringList uniqueSorted(QStringList values)
{
    for (auto &value : values) {
        value = normalizeFormatId(value);
    }

    values.removeAll(QString());
    values.removeDuplicates();
    values.sort();
    return values;
}

QStringList knownExtensions(const QString &formatId)
{
    if (formatId == QStringLiteral("markdown") || formatId == QStringLiteral("gfm") || formatId.startsWith(QStringLiteral("markdown_"))) {
        return {QStringLiteral("md"), QStringLiteral("markdown")};
    }
    if (formatId == QStringLiteral("html") || formatId == QStringLiteral("html4") || formatId == QStringLiteral("html5")) {
        return {QStringLiteral("html"), QStringLiteral("htm")};
    }
    if (formatId == QStringLiteral("docx")) {
        return {QStringLiteral("docx")};
    }
    if (formatId == QStringLiteral("pdf")) {
        return {QStringLiteral("pdf")};
    }
    if (formatId == QStringLiteral("pptx")) {
        return {QStringLiteral("pptx")};
    }
    if (formatId == QStringLiteral("epub") || formatId == QStringLiteral("epub2") || formatId == QStringLiteral("epub3")) {
        return {QStringLiteral("epub")};
    }
    if (formatId == QStringLiteral("odt")) {
        return {QStringLiteral("odt")};
    }
    if (formatId == QStringLiteral("rtf")) {
        return {QStringLiteral("rtf")};
    }
    if (formatId == QStringLiteral("latex")) {
        return {QStringLiteral("tex")};
    }
    if (formatId == QStringLiteral("typst")) {
        return {QStringLiteral("typ")};
    }
    if (formatId == QStringLiteral("json") || formatId == QStringLiteral("csljson")) {
        return {QStringLiteral("json")};
    }
    if (formatId == QStringLiteral("xml")) {
        return {QStringLiteral("xml")};
    }
    if (formatId == QStringLiteral("csv")) {
        return {QStringLiteral("csv")};
    }
    if (formatId == QStringLiteral("tsv")) {
        return {QStringLiteral("tsv")};
    }

    return {formatId};
}

} // namespace

PandocFormatService::PandocFormatService(QString pandocPath)
    : m_pandocPath(std::move(pandocPath))
{
    m_pandocPath = m_pandocPath.trimmed();
    if (m_pandocPath.isEmpty()) {
        m_pandocPath = QString::fromLatin1(DefaultPandocPath);
    }
}

QString PandocFormatService::pandocPath() const
{
    return m_pandocPath;
}

PandocFormatDiscoveryResult PandocFormatService::discover(int timeoutMs) const
{
    QString inputError;
    const auto inputFormats = runListCommand(QStringLiteral("--list-input-formats"), timeoutMs, &inputError);

    QString outputError;
    const auto outputFormats = runListCommand(QStringLiteral("--list-output-formats"), timeoutMs, &outputError);

    if (inputFormats.isEmpty() || outputFormats.isEmpty()) {
        const auto fallbackInputs = fallbackInputFormats();
        const auto fallbackOutputs = fallbackOutputFormats();
        return {
            false,
            mergePandocFormats(fallbackInputs, fallbackOutputs),
            fallbackInputs,
            fallbackOutputs,
            inputError.isEmpty() ? outputError : inputError,
        };
    }

    return {
        true,
        mergePandocFormats(inputFormats, outputFormats),
        inputFormats,
        outputFormats,
        QStringLiteral("Pandoc formats discovered."),
    };
}

QStringList PandocFormatService::fallbackInputFormats()
{
    return {QStringLiteral("markdown")};
}

QStringList PandocFormatService::fallbackOutputFormats()
{
    return supportedOutputFormats();
}

QStringList PandocFormatService::runListCommand(const QString &argument, int timeoutMs, QString *errorMessage) const
{
    const auto program = executableProgram();
    if (program.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Pandoc executable was not found.");
        }
        return {};
    }

    QProcess process;
    process.setProgram(program);
    process.setArguments({argument});
    process.start();

    if (!process.waitForStarted()) {
        if (errorMessage) {
            *errorMessage = process.errorString();
        }
        return {};
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        if (errorMessage) {
            *errorMessage = QStringLiteral("Pandoc format discovery timed out.");
        }
        return {};
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage) {
            *errorMessage = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        }
        return {};
    }

    return parseFormatLines(QString::fromLocal8Bit(process.readAllStandardOutput()));
}

QString PandocFormatService::executableProgram() const
{
    const QFileInfo pandocInfo(m_pandocPath);
    if (pandocInfo.isAbsolute()) {
        return pandocInfo.exists() && pandocInfo.isFile() ? m_pandocPath : QString();
    }

    return QStandardPaths::findExecutable(m_pandocPath);
}

QString formatLabel(const QString &formatId)
{
    const auto id = normalizeFormatId(formatId);
    if (id == QStringLiteral("html")) {
        return QStringLiteral("HTML");
    }
    if (id == QStringLiteral("docx")) {
        return QStringLiteral("Word 文档");
    }
    if (id == QStringLiteral("pdf")) {
        return QStringLiteral("PDF");
    }
    if (id == QStringLiteral("markdown")) {
        return QStringLiteral("Markdown");
    }
    if (id == QStringLiteral("pptx")) {
        return QStringLiteral("PowerPoint");
    }
    if (id == QStringLiteral("epub")) {
        return QStringLiteral("EPUB");
    }

    return id;
}

QString recommendedExtensionForFormat(const QString &formatId)
{
    const auto extensions = knownExtensions(normalizeFormatId(formatId));
    return extensions.isEmpty() ? normalizeFormatId(formatId) : extensions.first();
}

QStringList sortFormatsByPriority(const QStringList &formats, const QStringList &priorityFormats)
{
    const auto normalizedFormats = uniqueSorted(formats);
    QStringList sorted;

    for (const auto &priorityFormat : priorityFormats) {
        const auto normalizedPriority = normalizeFormatId(priorityFormat);
        if (normalizedFormats.contains(normalizedPriority) && !sorted.contains(normalizedPriority)) {
            sorted << normalizedPriority;
        }
    }

    for (const auto &format : normalizedFormats) {
        if (!sorted.contains(format)) {
            sorted << format;
        }
    }

    return sorted;
}

QVector<PandocFormat> mergePandocFormats(const QStringList &inputFormats, const QStringList &outputFormats)
{
    const auto readable = uniqueSorted(inputFormats);
    const auto writable = uniqueSorted(outputFormats);

    QSet<QString> allIds;
    for (const auto &format : readable) {
        allIds.insert(format);
    }
    for (const auto &format : writable) {
        allIds.insert(format);
    }

    QStringList sortedIds = allIds.values();
    sortedIds.sort();

    QVector<PandocFormat> formats;
    formats.reserve(sortedIds.size());

    for (const auto &id : sortedIds) {
        formats.push_back({
            id,
            formatLabel(id),
            knownExtensions(id),
            readable.contains(id),
            writable.contains(id),
        });
    }

    return formats;
}

} // namespace Md2Any
