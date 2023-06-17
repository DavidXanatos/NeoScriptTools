#include "Engine.h"

CEngine::CEngine(QObject* parent) 
	: QObject(parent) 
{
}

void CEngine::Alert(const QString& Message)
{
	qDebug() << Message;
	emit LogMessage(Message);
}

void CEngine::Sleep(quint32 ms)
{
	QElapsedTimer timer;
	timer.start();
	while (timer.elapsed() < ms)
		QCoreApplication::processEvents();
}