#pragma once

#include "conversion_result.h"
#include "conversion_task.h"

#include <QString>
#include <QStringList>

namespace Md2Any {

class PandocRunner
{
public:
    explicit PandocRunner(QString pandocPath = QString());

    QString pandocPath() const;
    ConversionResult checkPandocAvailable() const;
    ConversionResult validate(const ConversionTask &task) const;
    ConversionResult run(const ConversionTask &task, int timeoutMs = 60000) const;
    QStringList buildArguments(const ConversionTask &task) const;
    QStringList buildCommand(const ConversionTask &task) const;

private:
    QString m_pandocPath;
};

QString normalizedOutputFormat(const QString &format);

} // namespace Md2Any
