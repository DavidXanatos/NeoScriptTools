#pragma once
#include "Engine.h"
#include <QJSEngine>

#include "../V4ScriptDebugger/V4ScriptDebuggerBackend.h"


class CV4Engine : public CEngine
{
	Q_OBJECT
public:
	CV4Engine(QObject* parent = NULL);

	virtual bool RunScript(const QString& Script, const QString& FileName = "terminal");
	QObject* GetDebuggerBackend() { return m_pDebuggerBackend; }

protected:
	CV4EngineExt* m_pEngine;
	CV4ScriptDebuggerBackend* m_pDebuggerBackend;
};

