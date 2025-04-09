#include "QtUtility/python/py_process.hpp"

#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>

namespace QtUtility {

python::Output python::run(const QResource &script, const QStringList &args)
{
    python::Output output;

    if (!script.isValid()) {
        output.error = "Resource not found";
        return output;
    }

    if (QFileInfo(script.fileName()).suffix() != "py") {
        output.error = "Resource file is not a .py";
        return output;
    }

    QTemporaryFile tempFile;
    if (tempFile.open()) {
        tempFile.write(reinterpret_cast<const char *>(script.data()), script.size());
        tempFile.close();
    } else {
        output.error = "Failed to create temporary file";
        return output;
    }

    QProcess process;
    QStringList arguments;
    arguments << tempFile.fileName() << args;

    process.start("python", arguments);
    process.waitForFinished();

    output.output = process.readAllStandardOutput();
    output.error = process.readAllStandardError();
    return output;
}

} // namespace QtUtility