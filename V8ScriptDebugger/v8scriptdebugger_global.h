#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(V8SCRIPTDEBUGGER_LIB)
#  define V8SCRIPTDEBUGGER_EXPORT Q_DECL_EXPORT
# else
#  define V8SCRIPTDEBUGGER_EXPORT Q_DECL_IMPORT
# endif
#else
# define V8SCRIPTDEBUGGER_EXPORT
#endif
