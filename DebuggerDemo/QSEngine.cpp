#include "QSEngine.h"


CQSEngine::CQSEngine(QObject* parent) 
	: CEngine(parent) 
{
	m_pEngine = new QScriptEngine(this);

	m_pDebuggerBackend = new CJSScriptDebuggerBackend();
	m_pDebuggerBackend->attachTo(m_pEngine);

	QScriptValue Global = m_pEngine->globalObject();

	Global.setProperty("alert", m_pEngine->newFunction(FxAlert));
	Global.setProperty("sleep", m_pEngine->newFunction(FxSleep));
}

bool CQSEngine::RunScript(const QString& Script, const QString& FileName)
{
	QScriptValue ret = m_pEngine->evaluate(Script, FileName);
	emit EvalFinished(ret.toVariant());
	return true;
}

QScriptValue CQSEngine::FxAlert(QScriptContext* pContext, QScriptEngine* pEngine)
{
	((CQSEngine*)pEngine->parent())->Alert(pContext->argument(0).toString());
	return QScriptValue();
}

QScriptValue CQSEngine::FxSleep(QScriptContext* pContext, QScriptEngine* pEngine)
{
	((CQSEngine*)pEngine->parent())->Sleep(pContext->argument(0).toInt32());
	return QScriptValue();
}
