#include "V4Engine.h"

#include <private/qv4engine_p.h>
#include <private/qv4script_p.h>

QV4::ReturnedValue method_alert(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine* v4 = scope.engine;

    QString Result;
    for (int i = 0; i < argc; i++) {
        if (i > 0) Result.append(" ");
        Result.append(argv[i].toQStringNoThrow());
    }
    ((CEngine*)CV4EngineExt::getEngineByHandle(v4)->parent())->Alert(Result);

    return QV4::Encode::undefined();
}

QV4::ReturnedValue method_sleep(const QV4::FunctionObject* b, const QV4::Value* v, const QV4::Value* argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine* v4 = scope.engine;

    ((CEngine*)CV4EngineExt::getEngineByHandle(v4)->parent())->Sleep(argv[0].toNumber());

    return QV4::Encode::undefined();
}

CV4Engine::CV4Engine(QObject* parent) 
	: CEngine(parent) 
{
	m_pEngine = new CV4EngineExt(this); // the engine lives in its own thread

    m_pDebuggerBackend = new CV4ScriptDebuggerBackend(); // not this this remains in the main thread
    m_pDebuggerBackend->attachTo(m_pEngine);

	QV4::Scope scope(m_pEngine->handle());
	scope.engine->globalObject->defineDefaultProperty(QStringLiteral("alert"), method_alert);
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("sleep"), method_sleep);
}


bool CV4Engine::RunScript(const QString& Script, const QString& FileName)
{
    QJSValue ret = m_pEngine->evaluateScript(Script, FileName);
    emit EvalFinished(ret.toVariant());
    return true;
}