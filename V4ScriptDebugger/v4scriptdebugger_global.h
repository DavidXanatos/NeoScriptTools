#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(V4SCRIPTDEBUGGER_LIB)
#  define V4SCRIPTDEBUGGER_EXPORT Q_DECL_EXPORT
# else
#  define V4SCRIPTDEBUGGER_EXPORT Q_DECL_IMPORT
# endif
#else
# define V4SCRIPTDEBUGGER_EXPORT
#endif
