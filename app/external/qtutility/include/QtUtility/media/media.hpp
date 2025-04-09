#pragma once

#include <QIcon>
#include <QImage>
#include <QPixmap>

#include "QtUtility/export.hpp"

namespace QtUtility {
namespace media {

// recolor all non transparent pixels to the chosen color
QTUTILITY_EXPORT QIcon recolor(const QIcon &icon, const QColor &color, const QSize &size = QSize(256, 256));
QTUTILITY_EXPORT QPixmap recolor(const QPixmap &pixmap, const QColor &color);
QTUTILITY_EXPORT QImage recolor(const QImage &image, const QColor &color);

} // namespace media
} // namespace QtUtility