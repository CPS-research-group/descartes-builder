#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>

#include <mutex>

namespace data {
class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings &instance();

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key) const;
    // for testing purposes
    void printAll() const;

signals:
    void settingUpdated(const QString &key, const QVariant &value);

private:
    Settings();
    ~Settings();
    Settings(const Settings &) = delete;
    Settings &operator=(const Settings &) = delete;

    QSettings m_settings;
    mutable std::mutex m_mutex;
};
} // namespace data