/****************************************************************************
**
** Copyright (C) 2012 NeoLoader Team
** All rights reserved.
** Contact: XanatosDavid@gmil.com
**
** This file is part of the NeoScriptTools module for NeoLoader
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

#ifndef V8SCRIPTDEBUGGERFRONTEND_H
#define V8SCRIPTDEBUGGERFRONTEND_H

#include <QObject>
#include <QVariant>

#include "../NeoScriptTools/JSDebugging/JSScriptDebuggerFrontendInterface.h"
#include "V8DebugAdapter.h"

#include "v8scriptdebugger_global.h"

class QScriptDebugger;
class CV8ScriptDebuggerFrontendPrivate;
class V8SCRIPTDEBUGGER_EXPORT CV8ScriptDebuggerFrontend : public QObject, public CJSScriptDebuggerFrontendInterface, protected CV8DebugAdapter
{
    Q_OBJECT
public:
    CV8ScriptDebuggerFrontend(QObject *parent = 0);
    ~CV8ScriptDebuggerFrontend();

	virtual void connectTo(quint16 port = 9222);
	virtual void detach();

protected:
	virtual void processCommand(int id, const QVariantMap &command);

	virtual void onRequest(const QByteArray& arrJson);

	virtual void onEvent(const QVariantMap& Event);
	virtual void onResult(int id, const QVariantMap& Result);

private slots:
	void onReadyRead();

private:
	Q_DECLARE_PRIVATE(CV8ScriptDebuggerFrontend)
    Q_DISABLE_COPY(CV8ScriptDebuggerFrontend)
};

#endif
