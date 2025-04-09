#include "QtUtility/file/file.hpp"

#include <QDir>
#include <QDirIterator>

namespace QtUtility {

bool file::create(const QFileInfo &info, const bool &override)
{
    QFile file(info.filePath());
    if (file.exists()) {
        if (override)
            return file.resize(0);
        else
            return true;
    }
    info.absoluteDir().mkpath(".");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    file.close();
    return true;
}

QFileInfo file::getUniqueFile(const QFileInfo &info)
{
    QFileInfo result = info;
    int counter = 1;
    while (result.exists())
        result = QFileInfo(info.dir(),
                           QString("%1 (%2).%3")
                               .arg(info.baseName())
                               .arg(++counter)
                               .arg(info.completeSuffix()));
    return result;
}

QFileInfo file::createUnique(const QFileInfo &info)
{
    QFileInfo result = getUniqueFile(info);
    QFile file(result.filePath());
    info.absoluteDir().mkpath(".");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return QFileInfo();
    file.close();
    return result;
}

void file::copyAndReplaceFolderContents(const QDir &from, const QDir &to)
{
    QDirIterator it(from.absolutePath(),
                    QDir::AllEntries | QDir::Hidden | QDir::System,
                    QDirIterator::Subdirectories);
    const int absSourcePathLength = from.absolutePath().length();

    while (it.hasNext()) {
        it.next();
        const auto fileInfo = it.fileInfo();
        const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
        const QString constructedAbsolutePath = to.absolutePath() + subPathStructure;

        if (fileInfo.isDir()) {
            to.mkpath(constructedAbsolutePath);
        } else if (fileInfo.isFile()) {
            QFile::remove(constructedAbsolutePath);
            QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
        }
    }
}

} // namespace QtUtility