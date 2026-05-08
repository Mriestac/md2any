#include "settings_store.h"

#include "conversion_task.h"

#include <QSettings>
#include <utility>

namespace Md2Any {

namespace {

constexpr auto SettingsOrganization = "md2any";
constexpr auto SettingsApplication = "md2any";

QSettings makeSettings(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return QSettings(
            QString::fromLatin1(SettingsOrganization),
            QString::fromLatin1(SettingsApplication));
    }

    return QSettings(filePath, QSettings::IniFormat);
}

QString valueOrDefault(QSettings &settings, const char *key, const QString &fallback)
{
    const auto value = settings.value(QString::fromLatin1(key), fallback).toString().trimmed();
    return value.isEmpty() ? fallback : value;
}

QString normalizedFormatOrDefault(const QString &value, const QString &fallback)
{
    const auto normalized = value.trimmed().toLower();
    return normalized.isEmpty() ? fallback : normalized;
}

} // namespace

SettingsStore::SettingsStore() = default;

SettingsStore::SettingsStore(QString filePath)
    : m_filePath(std::move(filePath))
{
}

AppSettings defaultSettings()
{
    return {
        QString::fromLatin1(DefaultPandocPath),
        {},
        {},
        QString::fromLatin1(DefaultInputFormat),
        QString::fromLatin1(DefaultOutputFormat),
    };
}

AppSettings SettingsStore::load() const
{
    auto settings = makeSettings(m_filePath);
    const auto defaults = defaultSettings();
    const auto lastInputFormat = valueOrDefault(settings, "lastInputFormat", defaults.lastInputFormat);
    const auto lastOutputFormat = valueOrDefault(settings, "lastOutputFormat", defaults.lastOutputFormat);

    return {
        valueOrDefault(settings, "pandocPath", defaults.pandocPath),
        settings.value(QStringLiteral("lastInputDir"), defaults.lastInputDir).toString(),
        settings.value(QStringLiteral("lastOutputDir"), defaults.lastOutputDir).toString(),
        normalizedFormatOrDefault(lastInputFormat, defaults.lastInputFormat),
        normalizedFormatOrDefault(lastOutputFormat, defaults.lastOutputFormat),
    };
}

void SettingsStore::save(const AppSettings &settings) const
{
    auto store = makeSettings(m_filePath);

    store.setValue(QStringLiteral("pandocPath"), settings.pandocPath.trimmed().isEmpty()
        ? defaultSettings().pandocPath
        : settings.pandocPath.trimmed());
    store.setValue(QStringLiteral("lastInputDir"), settings.lastInputDir);
    store.setValue(QStringLiteral("lastOutputDir"), settings.lastOutputDir);
    store.setValue(
        QStringLiteral("lastInputFormat"),
        normalizedFormatOrDefault(settings.lastInputFormat, defaultSettings().lastInputFormat));
    store.setValue(
        QStringLiteral("lastOutputFormat"),
        normalizedFormatOrDefault(settings.lastOutputFormat, defaultSettings().lastOutputFormat));
    store.sync();
}

} // namespace Md2Any
