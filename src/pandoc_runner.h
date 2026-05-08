#pragma once

#include "conversion_result.h"
#include "conversion_task.h"

#include <QString>
#include <QStringList>

namespace Md2Any {

class PandocRunner
{
public:
    explicit PandocRunner(
        QString pandocPath = QString(),
        QStringList inputFormats = supportedInputFormats(),
        QStringList outputFormats = supportedOutputFormats());

    QString pandocPath() const;
    QStringList inputFormats() const;
    QStringList outputFormats() const;
    ConversionResult checkPandocAvailable() const;
    ConversionResult validate(const ConversionTask &task) const;
    ConversionResult run(const ConversionTask &task, int timeoutMs = 60000) const;
    QStringList buildArguments(const ConversionTask &task) const;
    QStringList buildCommand(const ConversionTask &task) const;

private:
    QString m_pandocPath;
    QStringList m_inputFormats;
    QStringList m_outputFormats;
};

QString normalizedInputFormat(const QString &format);
QString normalizedOutputFormat(const QString &format);

} // namespace Md2Any
