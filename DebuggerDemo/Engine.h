#pragma once
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QApplication>

class CEngine : public QObject
{
	Q_OBJECT
public:
	CEngine(QObject* parent = NULL);

	virtual void Alert(const QString& Message);
	virtual void Sleep(quint32 ms);

public slots:
	virtual bool RunScript(const QString& Script, const QString& FileName = "terminal") = 0;
	virtual QObject* GetDebuggerBackend() = 0;

signals:
	void LogMessage(const QString& Message);
	void EvalFinished(const QVariant& Result);

};

