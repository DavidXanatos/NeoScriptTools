#pragma once
#include "Engine.h"

class CEngineThread : public QThread
{
	Q_OBJECT
public:
	CEngineThread(CEngine* pEngine, QObject* parrent = NULL);
	~CEngineThread();

	virtual void RunScript(const QString& Script, const QString& FileName = "terminal");

	QObject* GetDebuggerBackend() { return m_pEngine->GetDebuggerBackend(); }

protected:
	CEngine* m_pEngine;
};
