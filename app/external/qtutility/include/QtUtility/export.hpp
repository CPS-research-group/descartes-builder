#pragma once
#include <QtCore/QtGlobal>

#if defined(QTUTILITY_LIBRARY)
#  define QTUTILITY_EXPORT Q_DECL_EXPORT
#else
#  define QTUTILITY_EXPORT Q_DECL_IMPORT
#endif