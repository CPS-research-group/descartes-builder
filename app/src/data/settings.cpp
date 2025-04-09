#include "data/settings.hpp"

namespace {

const std::map<QString, QVariant> DEFAULT_VALUES = {
    {"engine", "kedro"},
    {"engine timeout (minutes)", 5},
    {"default export format", ".dcb (Graph + data)"},
};

}

namespace data {
Settings &Settings::instance()
{
    static Settings instance;
    return instance;
}

Settings::Settings()
    : m_settings("CNRS@CREATE", "DesCartes Builder")
{
    // init the default values if they don't exist
    for (auto pair : DEFAULT_VALUES)
        if (!m_settings.contains(pair.first))
            m_settings.setValue(pair.first, pair.second);
}

Settings::~Settings() {}

void Settings::setValue(const QString &key, const QVariant &value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_settings.setValue(key, value);
    emit settingUpdated(key, value);
}

QVariant Settings::value(const QString &key) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (DEFAULT_VALUES.count(key) > 0)
        return m_settings.value(key, DEFAULT_VALUES.at(key));
    return m_settings.value(key);
}

void Settings::printAll() const
{
    for (auto &pair : DEFAULT_VALUES)
        qDebug() << pair.first << ": " << value(pair.first);
}

} // namespace data