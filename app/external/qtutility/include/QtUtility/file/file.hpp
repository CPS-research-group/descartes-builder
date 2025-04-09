#pragma once

#include <QFileInfo>

#include "QtUtility/export.hpp"

namespace QtUtility {
namespace file {

QTUTILITY_EXPORT bool create(const QFileInfo &info, const bool &override = false);
QTUTILITY_EXPORT QFileInfo getUniqueFile(const QFileInfo &info);
QTUTILITY_EXPORT QFileInfo createUnique(const QFileInfo &info);
QTUTILITY_EXPORT void copyAndReplaceFolderContents(const QDir &from, const QDir &to);

} // namespace file
} // namespace QtUtility