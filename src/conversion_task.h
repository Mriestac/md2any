#pragma once

#include <QString>
#include <QStringList>

namespace Md2Any {

inline constexpr auto DefaultPandocPath = "pandoc";
inline constexpr auto DefaultOutputFormat = "html";

inline QStringList supportedOutputFormats()
{
    return {QStringLiteral("html"), QStringLiteral("docx"), QStringLiteral("pdf")};
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
};

} // namespace Md2Any
