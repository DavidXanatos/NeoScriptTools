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

#ifndef CV4SCRIPTDEBUGGERBACKEND_H
#define CV4SCRIPTDEBUGGERBACKEND_H

#include "v4scriptdebugger_global.h"

#include <QObject>
#include <QVariant>

#include <QJSEngine>

class CV4DebugAgent;

class CV4ScriptDebuggerBackendPrivate;
class V4SCRIPTDEBUGGER_EXPORT CV4ScriptDebuggerBackend : public QObject
{
    Q_OBJECT
public:
	CV4ScriptDebuggerBackend(QObject *parent = 0);
    ~CV4ScriptDebuggerBackend();

	QVariant handleRequest(const QVariant& var);

	QVariantMap onCommand(int id, const QVariantMap& Command);
	void attachTo(class CV4EngineItf* engine);
//  void detach();

signals:
	void sendResponse(const QVariant& var);

public slots:
	void processRequest(const QVariant& var);

private slots:
    void debuggerPaused(CV4DebugAgent* debugger, int reason, const QString& fileName, int lineNumber);
    void evaluateFinished(const QJSValue& ret);
    void printTrace(const QString& Message);
	void invokeDebugger();

protected:
	virtual QVariant handleCustom(const QVariant& var) {return QVariant();}
	virtual void requestStart() {}

    void evalFinished(const QVariant& Value, const QString& Message = QString());
	
    QVariantMap scriptDelta();
    void clear();

private:
	Q_DISABLE_COPY(CV4ScriptDebuggerBackend)
    Q_DECLARE_PRIVATE(CV4ScriptDebuggerBackend)
};

#endif
