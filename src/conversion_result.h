#pragma once

#include <QString>
#include <QStringList>

namespace Md2Any {

struct ConversionResult
{
    bool success = false;
    QStringList command;
    int exitCode = -1;
    QString standardOutput;
    QString standardError;
    QString outputPath;
    QString message;
};

} // namespace Md2Any
