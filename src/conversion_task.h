#pragma once

#include <QString>
#include <QStringList>

namespace Md2Any {

inline constexpr auto DefaultPandocPath = "pandoc";
inline constexpr auto DefaultInputFormat = "markdown";
inline constexpr auto DefaultOutputFormat = "html";

inline QStringList supportedInputFormats()
{
    return {QStringLiteral("markdown")};
}

inline QStringList supportedOutputFormats()
{
    return {QStringLiteral("html"), QStringLiteral("docx"), QStringLiteral("pdf")};
}

inline bool isSupportedInputFormat(const QString &format)
{
    return supportedInputFormats().contains(format.trimmed().toLower());
}

inline bool isSupportedOutputFormat(const QString &format)
{
    return supportedOutputFormats().contains(format.trimmed().toLower());
}

struct ConversionTask
{
    QString inputPath;
    QString outputPath;
    QString outputFormat = QString::fromLatin1(DefaultOutputFormat);
    bool overwrite = false;
    QStringList extraArgs;
    QString inputFormat = QString::fromLatin1(DefaultInputFormat);
};

} // namespace Md2Any
