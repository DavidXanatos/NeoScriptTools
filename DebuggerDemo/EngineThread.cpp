#include "EngineThread.h"

void moveToThreadRecursively(QObject* pObject, QThread* pThread)
{
	pObject->moveToThread(pThread);
	foreach(QObject * pChild, pObject->children())
		moveToThreadRecursively(pChild, pThread);
}

CEngineThread::CEngineThread(CEngine* pEngine, QObject* parrent)
	: QThread(parrent)
{
	m_pEngine = pEngine;

	moveToThreadRecursively(m_pEngine, this);

	start();
}

CEngineThread::~CEngineThread()
{
	wait();
}

void CEngineThread::RunScript(const QString& Script, const QString& FileName)
{
	QMetaObject::invokeMethod(m_pEngine, "RunScript", Qt::QueuedConnection, Q_ARG(QString, Script), Q_ARG(QString, FileName));
}