/****************************************************************************
**
** Copyright (C) 2012 NeoLoader Team
** All rights reserved.
** Contact: XanatosDavid@gmil.com
**
** This file is part of the NeoScriptTools module for NeoLoader
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
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

#include "JSScriptDebuggerBackend.h"

#include "../debugging/qscriptdebuggerbackend_p.h"
#include "private/qobject_p.h"
#include <QEventLoop>

#include "../debugging/qscriptdebuggercommand_p.h"
#include "../debugging/qscriptdebuggerevent_p.h"
#include "../debugging/qscriptdebuggerresponse_p.h"
#include "../debugging/qscriptdebuggercommandexecutor_p.h"
#include "../debugging/qscriptbreakpointdata_p.h"
#include "../debugging/qscriptdebuggerobjectsnapshotdelta_p.h"

#include <QThread>
#include <QCoreApplication>


class CJSScriptDebuggerBackendPrivate: public QObjectPrivate, public QScriptDebuggerBackend
{
	Q_DECLARE_PUBLIC(CJSScriptDebuggerBackend)
public:

	void requestStart();

    void resume();

	QVariantList pendingEvents;
	int eventStackCounter;

protected:
    void event(const QScriptDebuggerEvent &event);

};

CJSScriptDebuggerBackend::CJSScriptDebuggerBackend(QObject *parent)
	: QObject(*new CJSScriptDebuggerBackendPrivate, parent)
{
	Q_D(CJSScriptDebuggerBackend);
	d->eventStackCounter = 0;
}

CJSScriptDebuggerBackend::~CJSScriptDebuggerBackend()
{
}

QVariant CJSScriptDebuggerBackend::handleRequest(const QVariant& var)
{
	Q_D(CJSScriptDebuggerBackend);

	QVariantMap in = var.toMap();
	if (in.contains("Control"))
	{
		if (in["Control"] == "PullEvent")
		{
			QVariantMap out;
			if (!d->pendingEvents.isEmpty())
				out["Event"] = d->pendingEvents.takeFirst();
			return out;
		}
		else if (in["Control"] == "Detach")
		{
			detach();
		}
	}
	else if (in.contains("Command"))
	{
		qint32 id = in["ID"].toUInt();
		QScriptDebuggerCommand command(QScriptDebuggerCommand::None);
		command.fromVariant(in["Command"]);

		QScriptDebuggerResponse response = d->commandExecutor()->execute(d, command);

		QVariantMap out;
		out["ID"] = id;
		out["Result"] = response.toVariant();
		return out;
	}
	else if (in.contains("Request"))
	{
		QVariantMap out;
		out["Response"] = handleCustom(in["Request"]);
		return out;
	}
	return QVariant();
}

void CJSScriptDebuggerBackend::processRequest(const QVariant& var)
{
	emit sendResponse(handleRequest(var));
}

void CJSScriptDebuggerBackendPrivate::event(const QScriptDebuggerEvent &event)
{
	pendingEvents.append(event.toVariant());

	eventStackCounter ++;
	while(eventStackCounter > 0)
	{
		QThread::msleep(10);
		QCoreApplication::processEvents();
	}
    
    doPendingEvaluate(/*postEvent=*/false);
}

void CJSScriptDebuggerBackendPrivate::resume()
{
	Q_Q(CJSScriptDebuggerBackend);
	if(eventStackCounter == 0)
		QMetaObject::invokeMethod(q, "onPendingEvaluate", Qt::QueuedConnection);
	eventStackCounter = 0;
}

void CJSScriptDebuggerBackendPrivate::requestStart()
{
	Q_Q(CJSScriptDebuggerBackend);
	q->requestStart();
}

void CJSScriptDebuggerBackend::onPendingEvaluate()
{
	Q_D(CJSScriptDebuggerBackend);
	d->doPendingEvaluate(/*postEvent=*/true);
}

void CJSScriptDebuggerBackend::attachTo(QScriptEngine* engine)
{
	Q_D(CJSScriptDebuggerBackend);
	d->attachTo(engine);
}

void CJSScriptDebuggerBackend::detach()
{
	Q_D(CJSScriptDebuggerBackend);
	d->resume();
	d->detach();
}

void CJSScriptDebuggerBackend::resume()
{
	Q_D(CJSScriptDebuggerBackend);
	d->resume();
}

bool CJSScriptDebuggerBackend::isEvaluating()
{
	Q_D(CJSScriptDebuggerBackend);
	return d->isEvaluating();
}