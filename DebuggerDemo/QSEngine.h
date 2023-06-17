#pragma once
#include "Engine.h"

#include <QScriptProgram>
#include <QScriptEngine>

#include "../../../../NeoScriptTools/JSDebugging/JSScriptDebuggerBackend.h"

class CQSEngine : public CEngine
{
	Q_OBJECT
public:
	CQSEngine(QObject* parent = NULL);

	virtual bool RunScript(const QString& Script, const QString& FileName = "terminal");
	QObject* GetDebuggerBackend() { return m_pDebuggerBackend; }

protected:
	QScriptEngine* m_pEngine;
	CJSScriptDebuggerBackend* m_pDebuggerBackend;

private:
	static QScriptValue		FxAlert(QScriptContext* pContext, QScriptEngine* pEngine);
	static QScriptValue		FxSleep(QScriptContext* pContext, QScriptEngine* pEngine);
};

