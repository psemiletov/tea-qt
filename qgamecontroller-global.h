#ifndef QGameController_GLOBAL_H
#define QGameController_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QGAMECONTROLLER_LIBRARY)
# define QGAMECONTROLLER_EXPORT Q_DECL_EXPORT
#else
# define QGAMECONTROLLER_EXPORT Q_DECL_IMPORT
#endif

#endif // QGameController_GLOBAL_H
