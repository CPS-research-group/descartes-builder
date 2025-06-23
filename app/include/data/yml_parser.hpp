#include <unordered_map>
#include <QString>

std::unordered_map<QString, QString> parseYml(const QString &content)
{
    std::unordered_map<QString, QString> map;
    for (const QString &line : content.split('\n')) {
        auto pair = line.split(':');
        if (pair.size() != 2)
            continue;
        map[pair[0].trimmed()] = pair[1].trimmed();
    }
    return map;
}