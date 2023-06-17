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

#include "V4ScriptDebuggerApi.h"
#include "V4ScriptDebuggerBackend.h"
#include "../NeoScriptTools/JSDebugging/JSScriptDebuggerFrontend.h"
#include "../NeoScriptTools/JSDebugging/JSScriptDebugger.h"

QObject* newV4ScriptDebuggerBackend(CV4EngineItf* engine)
{
	CV4ScriptDebuggerBackend* pDebuggerBackend = new CV4ScriptDebuggerBackend();
	pDebuggerBackend->attachTo(engine);
	return pDebuggerBackend;
}

QObject* newJSScriptDebuggerFrontend()
{
	return new CJSScriptDebuggerFrontend();
}

QMainWindow* newJSScriptDebugger(QObject* frontend)
{
	CJSScriptDebugger* pDebugger = new CJSScriptDebugger();
	pDebugger->attachTo((CJSScriptDebuggerFrontend*)frontend);
	return pDebugger;
}