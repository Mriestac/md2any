#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace Md2Any {

struct PandocFormat
{
    QString id;
    QString label;
    QStringList extensions;
    bool canRead = false;
    bool canWrite = false;
};

struct PandocFormatDiscoveryResult
{
    bool success = false;
    QVector<PandocFormat> formats;
    QStringList inputFormats;
    QStringList outputFormats;
    QString message;
};

class PandocFormatService
{
public:
    explicit PandocFormatService(QString pandocPath = QString());

    QString pandocPath() const;
    PandocFormatDiscoveryResult discover(int timeoutMs = 10000) const;

    static QStringList fallbackInputFormats();
    static QStringList fallbackOutputFormats();

private:
    QStringList runListCommand(const QString &argument, int timeoutMs, QString *errorMessage) const;
    QString executableProgram() const;

    QString m_pandocPath;
};

QString formatLabel(const QString &formatId);
QString recommendedExtensionForFormat(const QString &formatId);
QStringList sortFormatsByPriority(const QStringList &formats, const QStringList &priorityFormats);
QVector<PandocFormat> mergePandocFormats(const QStringList &inputFormats, const QStringList &outputFormats);

} // namespace Md2Any
