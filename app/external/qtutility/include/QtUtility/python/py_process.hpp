#pragma once

#include <QResource>
#include <QString>
#include <QStringList>

namespace QtUtility {
namespace python {

struct Output
{
    QString output;
    QString error;
};

Output run(const QResource &script, const QStringList &args = {});

} // namespace python
} // namespace QtUtility