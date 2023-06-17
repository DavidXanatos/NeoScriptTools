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

#include "V4DebugJobs.h"

#include "V4DebugHandler.h"
#include "V4DebugAgent.h"

#include <private/qv4script_p.h>
#include <private/qqmlcontext_p.h>
#include <private/qv4qmlcontext_p.h>
#include <private/qv4qobjectwrapper_p.h>
#include <private/qqmldebugservice_p.h>
#include <private/qv4jscall_p.h>
#include <private/qv4objectiterator_p.h>


////////////////////////////////////////////////////////////////////////////////////
// CV4ScopeJob
//

CV4GetPropsJob::CV4GetPropsJob(CV4DebugHandler* handler, UV4Handle handle) :
    handler(handler), handle(handle), success(false)
{
}

void CV4GetPropsJob::run()
{
    QV4::Scope scope(handler->engine());

    if (handle.type == UV4Handle::eObject)
    {
        if (handler->isValidRef(handle.ref)) {
            result = handler->lookupRef(handle.ref);
            success = true;
        }
    }
    else if (handle.type == UV4Handle::eScope)
    {
        QV4::Scoped<QV4::ExecutionContext> ctxt(scope, CV4DebugAgent::findScope(CV4DebugAgent::findContext(handler->engine(), handle.frame), handle.scope));
        if (ctxt && ctxt->d()->type == QV4::Heap::ExecutionContext::Type_CallContext) {
            QV4::ScopedValue v(scope);
            QV4::Heap::InternalClass* ic = ctxt->internalClass();
            for (uint i = 0; i < ic->size; ++i) {
                QString name = ic->keyAt(i);
                v = static_cast<QV4::Heap::CallContext*>(ctxt->d())->locals[i];
                uint ref = handler->addRef(v);
                result.properties.append(SV4Property(handler->getObject(v, ref), name));
            }
            success = true;
        }
    }
    else if (handle.type == UV4Handle::eThis)
    {
        QV4::CppStackFrame* frame = CV4DebugAgent::findFrame(handler->engine(), handle.frame);
        if (frame) {
            QV4::ScopedValue v(scope, frame->thisObject());
            uint ref = handler->addRef(v);
            result = handler->getObject(v, ref);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// CV4SetValueJob
//

CV4SetValueJob::CV4SetValueJob(CV4DebugHandler* handler, UV4Handle handle, const QString& name, const SV4Value& value)
    :handler(handler), handle(handle), name(name), value(value), success(false)
{
}

void CV4SetValueJob::run()
{
    QV4::Scope scope(handler->engine());

    QV4::ScopedValue v(scope);
    if (value.type == "undefined")
        v = QV4::Encode::undefined();
    else if (value.type == "null")
        v = QV4::Encode::null();
    else if (value.type == "boolean")
        v = QV4::Encode(value.data.toBool());
    else if (value.type == "number") 
        v = QV4::Encode(value.data.toDouble());
    else if (value.type == "string") 
        v = handler->engine()->newString(value.data.toString());
    else if (value.ref != -1)
        v = handler->getValue(value.ref);
        
    if (handle.type == UV4Handle::eObject)
    {
        QV4::ScopedObject o(scope, handler->getValue(handle.ref));
        if (o->as<QV4::ArrayObject>() != NULL) {
            o->put(name.toInt(), v);
        } 
        else if (o->as<QV4::Object>() != NULL) {
            QV4::ScopedString propName(scope);
            propName = handler->engine()->newString(name);
            o->put(propName, v);
        }
        success = true;
    }
    else if (handle.type == UV4Handle::eScope)
    {
        QV4::Scoped<QV4::ExecutionContext> ctxt(scope, CV4DebugAgent::findScope(CV4DebugAgent::findContext(handler->engine(), handle.frame), handle.scope));
        if (ctxt) {
            QV4::ScopedString propName(scope);
            propName = handler->engine()->newString(name);
            ctxt->setProperty(propName, v);
            success = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// CV4ScriptJob
//

CV4ScriptJob::CV4ScriptJob(QV4::ExecutionEngine* engine, const QString& program, int frameNr/*, int context*/) :
    engine(engine), frameNr(frameNr), program(program), //context(context),
    resultIsException(false)
{
}

void CV4ScriptJob::run()
{
    QV4::Scope scope(engine);

    QV4::ScopedValue result = exec(engine, scope, program, frameNr/*, context*/);

    if (scope.engine->hasException) {
        result = scope.engine->catchException();
        resultIsException = true;
    }

    handleResult(result);
}

QV4::ScopedValue CV4ScriptJob::exec(QV4::ExecutionEngine* engine, QV4::Scope& scope, const QString& program, int frameNr/*, int context*/)
{
    QV4::ScopedContext ctx(scope, engine->currentStackFrame ? engine->currentContext() : engine->scriptContext());

    QV4::CppStackFrame* frame = CV4DebugAgent::findFrame(engine, frameNr);
    if (frameNr > 0 && frame)
        ctx = frame->context();

    QV4::Script script(ctx, QV4::Compiler::ContextType::Eval, program);
    script.strictMode = (frame ? frame->v4Function : engine->globalCode)->isStrict();
    script.inheritContext = true;
    script.parse();
    QV4::ScopedValue result(scope);
    if (scope.engine->hasException) {
        //QV4::ScopedValue exceptionValue(scope, *engine->exceptionValue);
        result = engine->catchException();
    } else if (frame) {
        QV4::ScopedValue thisObject(scope, frame->thisObject());
        result = script.run(thisObject);
    } else
        result = script.run();
    return result;
}

////////////////////////////////////////////////////////////////////////////////////
// CV4RunScriptJob
//

CV4RunScriptJob::CV4RunScriptJob(QV4::ExecutionEngine* engine, CV4DebugHandler* handler, const QString& program, int frameNr/*, int context*/) :
    CV4ScriptJob(engine, program, frameNr/*, context*/), handler(handler)
{
}

void CV4RunScriptJob::handleResult(QV4::ScopedValue& value)
{
    if (hasExeption())
        exception = value->toQStringNoThrow();
    result = handler->lookupRef(handler->addRef(value));
}
