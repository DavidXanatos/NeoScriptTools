/****************************************************************************
**
** Copyright (C) 2023 David Xanatos (xanasoft.com) All rights reserved.
** Contact: XanatosDavid@gmil.com
**
**
** To use the V4ScriptTools in a commercial project, you must obtain
** an appropriate business use license.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
****************************************************************************/

#include "V4EngineExt.h"

#include <private/qv4engine_p.h>
#include <private/qv4debugging_p.h>
#include <private/qv4objectiterator_p.h>
#include <private/qv4string_p.h>
#include <private/qv4script_p.h>
#include <private/qqmlbuiltinfunctions_p.h>
#include <private/qqmldebugservice_p.h>

static QMutex g_engineMutex;
static QMap<void*, CV4EngineExt*> g_engineMap;

CV4EngineExt* CV4EngineExt::getEngineByHandle(void* handle)
{
    QMutexLocker locker(&g_engineMutex);
    return g_engineMap.value(handle);
}

static QV4::ReturnedValue printCall(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc);
static QV4::ReturnedValue debuggerCall(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc);
static QV4::ReturnedValue evalCall(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc);

CV4EngineExt::CV4EngineExt(QObject* parent) 
    : QJSEngine(parent)
{
    QV4::Scope scope(handle());

    // provide a print function to write to console
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("print"), printCall);
    // provide ability to invoke the debugger
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("_debugger"), debuggerCall);
    // overwrite the eval function with our own copy which traces the scripts
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("eval"), evalCall);

    QMutexLocker locker(&g_engineMutex);
    g_engineMap.insert(handle(), this);
}

CV4EngineExt::~CV4EngineExt()
{
    QMutexLocker locker(&g_engineMutex);
    g_engineMap.remove(handle());
}

QJSValue CV4EngineExt::evaluateScript(const QString& program, const QString& fileName, int lineNumber)
{
    QJSValue ret = QJSEngine::evaluate(program, trackScript(program, fileName, lineNumber), lineNumber);
    emit evaluateFinished(ret);
    return ret;
}

QString CV4EngineExt::trackScript(const QString& program, const QString& fileName, int lineNumber)
{
    QString Name = QUrl(fileName).fileName();
    QString FileName = Name;
    for (int i = 0; m_ScriptIDs.contains(FileName.toLower());)
        FileName = Name + " (" + QString::number(++i) + ")";
    m_ScriptIDs.insert(FileName.toLower(), m_Scripts.count());
    m_Scripts.append(SScript{ FileName, lineNumber, program });
    return FileName;
}

QV4::ReturnedValue printCall(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine* v4 = scope.engine;

    QString Result;
    for (int i = 0; i < argc; i++) {
        if (i > 0) Result.append(" ");
        Result.append(argv[i].toQStringNoThrow());
    }

    QMutexLocker locker(&g_engineMutex);
    emit g_engineMap.value(v4)->printTrace(Result);

    return QV4::Encode::undefined();
}

QV4::ReturnedValue debuggerCall(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine* v4 = scope.engine;

    QMutexLocker locker(&g_engineMutex);
    emit g_engineMap.value(v4)->invokeDebugger();

    return QV4::Encode::undefined();
}

QV4::ReturnedValue evalCall(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine* v4 = scope.engine;

    if (argc < 1)
        return QV4::Encode::undefined();

    QV4::String* scode = argv[0].stringValue();
    if (!scode)
        return argv[0].asReturnedValue();

    QMutexLocker locker(&g_engineMutex);
    QJSValue ret = g_engineMap.value(v4)->evaluateScript(scode->toQStringNoThrow(), "eval code");
    return g_engineMap.value(v4)->fromScriptValue<QV4::ReturnedValue>(ret);
}