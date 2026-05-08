#pragma once

#include <QString>

namespace Md2Any {

struct AppSettings
{
    QString pandocPath;
    QString lastInputDir;
    QString lastOutputDir;
    QString lastOutputFormat;
};

class SettingsStore
{
public:
    SettingsStore();
    explicit SettingsStore(QString filePath);

    AppSettings load() const;
    void save(const AppSettings &settings) const;

private:
    QString m_filePath;
};

AppSettings defaultSettings();

} // namespace Md2Any
