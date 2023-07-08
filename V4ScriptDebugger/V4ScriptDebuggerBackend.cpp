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

#include "V4ScriptDebuggerBackend.h"

#include "private/qobject_p.h"
#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <private/qv4engine_p.h>
#include <private/qv4debugging_p.h>
#include <private/qv4objectiterator_p.h>
#include <private/qv4string_p.h>
#include <private/qqmlbuiltinfunctions_p.h>
#include <private/qqmldebugservice_p.h>

#include "V4DebugAgent.h"
#include "V4DebugHandler.h"
#include "V4DebugJobs.h"

#include "V4ScriptDebuggerApi.h"

class CV4ScriptDebuggerBackendPrivate : public QObjectPrivate
{
	Q_DECLARE_PUBLIC(CV4ScriptDebuggerBackend)
public:

	CV4EngineItf*			engine;
	QPointer<CV4DebugAgent>	debugger;
	CV4DebugHandler*		handler;

	QVariantList			pendingEvents;

	QSet<qint64>			checkpointScripts;
	QSet<qint64>			previousCheckpointScripts;

	int						nextScriptObjectSnapshotId;
	QMap<int, struct SV4Object*> scriptObjectSnapshots;

	int						nextScriptValueIteratorId;
	QMap<int, struct SV4ValueIterator*> scriptValueIterators;
};

CV4ScriptDebuggerBackend::CV4ScriptDebuggerBackend(QObject *parent)
	: QObject(*new CV4ScriptDebuggerBackendPrivate, parent)
{
}

CV4ScriptDebuggerBackend::~CV4ScriptDebuggerBackend()
{
	clear();
}

void CV4ScriptDebuggerBackend::clear()
{
	Q_D(CV4ScriptDebuggerBackend);

	d->checkpointScripts.clear();
	d->previousCheckpointScripts.clear();

	foreach(SV4Object * snap, d->scriptObjectSnapshots)
		delete snap;

	foreach(SV4ValueIterator * iter, d->scriptValueIterators)
		delete iter;
}

QVariant CV4ScriptDebuggerBackend::handleRequest(const QVariant& var)
{
	Q_D(CV4ScriptDebuggerBackend);

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
		
		QVariantMap out;
		out["ID"] = id;
		out["Result"] = onCommand(id, in["Command"].toMap());
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

void CV4ScriptDebuggerBackend::processRequest(const QVariant& var)
{
	emit sendResponse(handleRequest(var));
}

QVariantMap CV4ScriptDebuggerBackend::onCommand(int id, const QVariantMap& Command)
{
	Q_D(CV4ScriptDebuggerBackend);

	QVariantMap Response;

	QString typeStr = Command["type"].toString();
	QVariantMap Attributes = Command["attributes"].toMap();

#ifdef _DEBUG
	QJsonDocument doc(QJsonValue::fromVariant(Attributes).toObject());
	QByteArray Temp = doc.toJson();
	//qDebug() << "cmd: " << typeStr;
#endif

	if (!d->debugger) {
		Response["error"] = "DetachedError";
		return Response;
	}

	//
	// Note: The debug agant must be in the same thread as the engine 
	//	so we follow it whenever needed
	//
	if (d->debugger->thread() != d->engine->self()->thread()) {
		d->debugger->moveToThread(d->engine->self()->thread());
		qDebug() << "V4DebugAgent moved to engine's thread";
	}

	
	if (typeStr == "Interrupt")
	{
		d->debugger->pause();
	}
	else if (typeStr == "Continue" || typeStr == "StepInto" || typeStr == "StepOver" || typeStr == "StepOut" || typeStr == "Resume")
	{
		CV4DebugAgent::Stepping stepping = CV4DebugAgent::NotStepping;
		if (typeStr == "StepInto")
			stepping = CV4DebugAgent::StepIn;
		else if (typeStr == "StepOver")
			stepping = CV4DebugAgent::StepOver;
		else if (typeStr == "StepOut")
			stepping = CV4DebugAgent::StepOut;
		d->debugger->resume(stepping);
		Response["async"] = true;
	}
	
	else if (typeStr == "RunToLocation" || typeStr == "RunToLocationByID")
	{
		int lineNumber = Attributes["lineNumber"].toInt();
		QString fileName;
		if (typeStr == "RunToLocationByID") {
			quint64 scriptId = Attributes["scriptId"].toULongLong();
			fileName = d->engine->getScriptName(scriptId);
		} else
			fileName = Attributes["fileName"].toString();
		d->debugger->runUntil(fileName, lineNumber);
		Response["async"] = true;
	}
	
	else if (typeStr == "Evaluate")
	{
		int contextIndex = Attributes["contextIndex"].toInt();
		QString fileName = Attributes["fileName"].toString();
		int lineNumber = Attributes["lineNumber"].toInt();
		QString program = Attributes["program"].toString();
		
		if (d->debugger->isPaused())
		{
			int frameNr = 0; // todo

			// Note: this mode is blocking - use only fast to evaluate expressions !!!
			CV4RunScriptJob job(d->debugger->engine(), d->handler, program, frameNr/*, -1*/);
			d->debugger->runJobInEngine(&job);
			evalFinished(job.returnValue().toVariant(), job.exceptionMessage());
		}
		else
		{
			// Note: this mode is not blocking
			QMetaObject::invokeMethod(d->engine->self(), "evaluateScript", Qt::QueuedConnection, Q_ARG(QString, program), Q_ARG(QString, fileName), Q_ARG(int, lineNumber));
		}
		Response["async"] = true;
	}
	else if (typeStr == "ForceReturn") // Used only in console commands
	{
		// does not seam to be supported by the V4 engine
	}

	else if (typeStr == "SetBreakpoint")
	{
		QVariantMap in = Attributes["breakpointData"].toMap();

		SV4Breakpoint bp;
		bp.fromVariant(in);
		if (quint64 scriptId = in["scriptId"].toLongLong())
			bp.fileName = d->engine->getScriptName(scriptId);

		Response["result"] = d->debugger->setBreakpoint(bp);
	}
	else if (typeStr == "DeleteBreakpoint")
	{
		d->debugger->deleteBreakpoint(Attributes["breakpointId"].toInt());
	}
	else if (typeStr == "DeleteAllBreakpoints")
	{
		d->debugger->deleteAllBreakpoints();
	}
	else if (typeStr == "GetBreakpoints")
	{
		QVariantList result;
		QMap<int, SV4Breakpoint> breakpoints = d->debugger->getBreakpoints();
		for (QMap<int, SV4Breakpoint>::iterator I = breakpoints.begin(); I != breakpoints.end(); ++I)
		{
			QVariantMap out = I.value().toVariant();
			out["id"] = I.key();
			out["scriptId"] = d->engine->getScriptId(I.value().fileName);
			result.append(out);
		}
		Response["result"] = result;
		Response["type"] = "QScriptBreakpointMap";
	}
	else if (typeStr == "GetBreakpointData")
	{
		QMap<int, SV4Breakpoint> breakpoints = d->debugger->getBreakpoints();
		auto I = breakpoints.find(Attributes["breakpointId"].toInt());
		if (I != breakpoints.end())
		{
			QVariantMap out = I.value().toVariant();
			out["id"] = I.key();
			out["scriptId"] = d->engine->getScriptId(I.value().fileName);
			Response["result"] = out;
			Response["type"] = "QScriptBreakpointData";
		}  else
			Response["error"] = "InvalidBreakpointID";
	}
	else if (typeStr == "SetBreakpointData")
	{
		QVariantMap in = Attributes["breakpointData"].toMap();

		SV4Breakpoint bp;
		bp.fromVariant(in);
		if (quint64 scriptId = in["scriptId"].toLongLong())
			bp.fileName = d->engine->getScriptName(scriptId);

		if(!d->debugger->updateBreakpoint(Attributes["breakpointId"].toInt(), bp))
			Response["error"] = "InvalidBreakpointID";
	}

	else if (typeStr == "GetScriptData")
	{
		quint64 scriptId = Attributes["scriptId"].toULongLong();
		if (scriptId >= d->engine->getScriptCount()) {
			Response["error"] = "InvalidScriptID";
			return Response;
		}

		QVariantMap Result;
		Result["contents"] = d->engine->getScriptSource(scriptId);
		Result["fileName"] = d->engine->getScriptName(scriptId);
		Result["baseLineNumber"] = d->engine->getScriptLineNumber(scriptId);
		//Result["timeStamp"] = .toLongLong();

		Response["result"] = Result;
		Response["type"] = "QScriptScriptData";
	}
	else if (typeStr == "ResolveScript") // used only in console commands
	{
		Response["result"] = d->engine->getScriptId(Attributes["fileName"].toString());
	}
	else if (typeStr == "GetScripts") // used only in console commands: .info scripts
	{
		QVariantList Scripts;
		//for(int i=0; i < d->engine->getScriptCount(); i++)
		foreach(const QString& fileName, d->debugger->getCurrentScripts())
		{
			int i = d->engine->getScriptId(fileName);
			QVariantMap Result;
			Result["id"] = i;
			Result["contents"] = d->engine->getScriptSource(i);
			Result["fileName"] = d->engine->getScriptName(i);
			Result["baseLineNumber"] = d->engine->getScriptLineNumber(i);

			Scripts.append(Result);
		}
		Response["result"] = Scripts;
		Response["type"] = "QScriptScriptMap";
	}
	else if (typeStr == "ScriptsCheckpoint")
	{
		d->previousCheckpointScripts = d->checkpointScripts;
		d->checkpointScripts.clear();
		//for(int i=0; i < d->engine->getScriptCount(); i++)
		//	d->checkpointScripts.insert(i);
		foreach(const QString& fileName, d->debugger->getCurrentScripts())
			d->checkpointScripts.insert(d->engine->getScriptId(fileName));

		Response["result"] = scriptDelta();
		Response["type"] = "QScriptScriptsDelta";
	}
	else if (typeStr == "GetScriptsDelta")
	{
		Response["result"] = scriptDelta();
		Response["type"] = "QScriptScriptsDelta";
	}

	else if (typeStr == "GetBacktrace") // used only in console commands: .backtrace
	{
		QVector<QV4::StackFrame> frames = d->debugger->stackTrace();

		QStringList Backtrace;
		foreach(const QV4::StackFrame& entry, frames)
			Backtrace.append(QString("%1() at %2:%3").arg(entry.function.isEmpty() ? "<anonymous>" : entry.function).arg(QUrl(entry.source).fileName()).arg(entry.line));
		Response["result"] = Backtrace;
	}
	else if (typeStr == "GetContextCount") // used only in console commands
	{
		QVector<QV4::StackFrame> frames = d->debugger->stackTrace();

		Response["result"] = frames.count();
	}

	else if (typeStr == "GetContextInfo")
	{
		QVector<QV4::StackFrame> frames = d->debugger->stackTrace();

		int frameNr = Attributes["contextIndex"].toInt();
		if(frameNr >= frames.size())
			Response["error"] = "InvalidContextIndex";
		else
		{
			QV4::StackFrame& frame = frames[frameNr];

			QVariantMap Result;
			Result["scriptId"] = d->engine->getScriptId(QUrl(frame.source).fileName());
			Result["lineNumber"] = frame.line;
			Result["columnNumber"] = frame.column;

			Result["fileName"] = QUrl(frame.source).fileName();
			Result["functionName"] = frame.function;

			//QJsonArray scopes;
			//QV4::Scope scope(d->debugger->engine());
			//for (QV4::ScopedContext ctxt(scope, CV4DebugAgent::findContext(d->debugger->engine(), frameNr)); ctxt; ctxt = ctxt->d()->outer)
			//{
			//	QV4::CallContext* cCtxt = ctxt->asCallContext();
			//	if (cCtxt && cCtxt->d()->activation) 
			//	{
			//		QV4::ScopedValue o(scope, ctxt->d()->activation);
			//		Response["receiver"] = d->handler->addRef(o);
			//		break;
			//	}
			//}

			Response["result"] = Result;
			Response["type"] = "QScriptDebuggerContextInfo";
		}
	}
	else if (typeStr == "GetContextState")
	{
		//int frameNr = Attributes["contextIndex"].toInt();
		Response["result"] = d->debugger->engine()->hasException ? 1 : 0;
	}
	else if (typeStr == "GetContextID")
	{
		//int frameNr = Attributes["contextIndex"].toInt();
		Response["result"] = 0;
	}
	else if (typeStr == "ContextsCheckpoint")
	{
		QVariantMap Result;
		Result["added"] = QVariantList();
		Result["removed"] = QVariantList();
		Response["result"] = Result;
		Response["type"] = "QScriptContextsDelta";
	}
	else if (typeStr == "GetThisObject")
	{
		int frameNr = Attributes["contextIndex"].toInt();

		UV4Handle Handle = { 0 };
		Handle.type = UV4Handle::eThis;
		Handle.frame = frameNr;

		CV4GetPropsJob job(d->handler, Handle);
		d->debugger->runJobInEngine(&job);
		SV4Object object = job.returnValue();
		
		Handle.type = UV4Handle::eObject;
		Handle.ref = object.ref;

		QVariantMap Value;
		Value["type"] = "ObjectValue";
		Value["value"] = Handle.value;
		Response["result"] = Value;
		Response["type"] = "QScriptDebuggerValue";
	}
	else if (typeStr == "GetScopeChain")
	{
		int frameNr = Attributes["contextIndex"].toInt();

		QMap<QString, int> NameCtr;

		QVariantList Result;
		foreach(const SV4Scope& scope, d->debugger->getScopes(frameNr)) {

			UV4Handle Scope = { 0 };
			Scope.type = UV4Handle::eScope;
			Scope.frame = frameNr;
			Scope.scope = scope.index;

			QString Name = scope.type;
			int i = NameCtr[Name]++;
			if (i > 0)
				Name += QString(" (%1)").arg(i);

			QVariantMap Value;
			Value["type"] = "ObjectValue";
			Value["value"] = Scope.value;

			QVariantMap Property;
			Property["name"] = Name;
			Property["value"] = Value;
			Property["flags"] = 0;

			Result.append(Property);
		}

		Response["result"] = Result;
		//Response["type"] = "QScriptDebuggerValueList";
		Response["type"] = "QScriptDebuggerValuePropertyList";
	}
	
	else if (typeStr == "GetActivationObject") // used only in console commands: .info locals
	{
		int frameNr = Attributes["contextIndex"].toInt();

		UV4Handle Scope = { 0 };
		Scope.frame = frameNr;
		
		foreach(const SV4Scope& scope, d->debugger->getScopes(frameNr)) {
			if (scope.type == "CallContext") {
				Scope.type = UV4Handle::eScope;
				Scope.scope = scope.index;
				break;
			}
		}

		QVariantMap Value;
		Value["type"] = "ObjectValue";
		Value["value"] = Scope.value;

		Response["result"] = Value;
		Response["type"] = "QScriptDebuggerValue";
	}

	else if (typeStr == "GetPropertyExpressionValue") // irrelevant used only for tooltips
		; 
	else if (typeStr == "GetCompletions") // irrelevant used only for autocomplete
		; 
		
	else if (typeStr == "NewScriptObjectSnapshot")
	{
		int snap_id = d->nextScriptObjectSnapshotId;
		++d->nextScriptObjectSnapshotId;
		d->scriptObjectSnapshots.insert(snap_id, new SV4Object());
		Response["result"] = snap_id;
	}
	else if (typeStr == "ScriptObjectSnapshotCapture")
	{
		QVariantMap value = Attributes["scriptValue"].toMap();
		Q_ASSERT(value["type"] == "ObjectValue"); // as provided by GetScopeChain
		UV4Handle Handle = { value["value"].toULongLong() };

		int snap_id = Attributes["snapshotId"].toInt();
		SV4Object* snap = d->scriptObjectSnapshots.value(snap_id);
		Q_ASSERT(snap != 0);
		if (!snap) {
			Response["error"] = "InvalidArgumentIndex";
			return Response;
		}
		snap->handle = Handle;

		CV4GetPropsJob job(d->handler, Handle);
		d->debugger->runJobInEngine(&job);
		SV4Object object = job.returnValue();

		QMap<QString, SV4Property> currProps;
		QHash<QString, int> propertyNameToIndex;
		for (int i = 0; i < object.properties.size(); i++)
		{
			const SV4Property& value = object.properties[i];
			currProps.insert(value.name, value);
			propertyNameToIndex.insert(value.name, i);
		}

		QSet<QString> prevSet;
		for (int i = 0; i < snap->properties.size(); ++i)
			prevSet.insert(snap->properties.at(i).name);
		QStringList currProps_keys = currProps.keys();
		QSet<QString> currSet = QSet<QString>(currProps_keys.begin(), currProps_keys.end());
		QSet<QString> removedPropertiesSet = prevSet - currSet;
		QSet<QString> addedPropertiesSet = currSet - prevSet;
		QSet<QString> maybeChangedPropertiesSet = currSet & prevSet;

		QMap<int, SV4Property> addedPropertiesMap;
		for (QSet<QString>::const_iterator I = addedPropertiesSet.constBegin(); I != addedPropertiesSet.constEnd(); I++) {
			int idx = propertyNameToIndex[*I];
			addedPropertiesMap[idx] = currProps[*I];
		}
		
		QList<SV4Property> changedPropertiesList;
		for (QSet<QString>::const_iterator I = maybeChangedPropertiesSet.constBegin(); I != maybeChangedPropertiesSet.constEnd(); I++) {
			const SV4Property& p1 = currProps[*I];
			SV4Property p2 = SV4Property();
			for (int i = 0; i < snap->properties.size(); ++i) {
				if (snap->properties.at(i).name == *I) {
					p2 = snap->properties.at(i);
					break;
				}
			}

			if (p1.type != p2.type || p1.data != p2.data)
				changedPropertiesList.append(p1);
		}

		snap->properties = currProps.values().toVector();
		

		QVariantMap result;

		result["removedProperties"] = QStringList(removedPropertiesSet.begin(), removedPropertiesSet.end());

		QVariantList changedProperties;
		foreach(const SV4Property & value, changedPropertiesList)
			changedProperties.append(value.toVariant());
		result["changedProperties"] = changedProperties;

		QVariantList addedProperties;
		foreach(const SV4Property& value, addedPropertiesMap)
			addedProperties.append(value.toVariant());
		result["addedProperties"] = addedProperties;

		Response["result"] = result;
		Response["type"] = "QScriptDebuggerObjectSnapshotDelta";
	}
	else if (typeStr == "ScriptValueToString") // used only in console commands
	{
		QVariantMap value = Attributes["scriptValue"].toMap();
		Q_ASSERT(value["type"] == "ObjectValue");
		UV4Handle Handle = { value["value"].toULongLong() };

		// todo

		Response["result"] = "TODO: not implemented";
	}
	else if (typeStr == "NewScriptValueIterator") // used only in console commands
	{
		QVariantMap value = Attributes["scriptValue"].toMap();
		Q_ASSERT(value["type"] == "ObjectValue"); // as provided by GetScopeChain
		UV4Handle Handle = { value["value"].toULongLong() };

		int iter_id = d->nextScriptValueIteratorId;
		++d->nextScriptValueIteratorId;
		SV4ValueIterator* iter = new SV4ValueIterator();
		d->scriptValueIterators.insert(id, iter);

		CV4GetPropsJob job(d->handler, Handle);
		d->debugger->runJobInEngine(&job);
		iter->snapshot = job.returnValue();

		Response["result"] = id;
	}
	else if (typeStr == "DeleteScriptObjectSnapshot")
	{
		int snap_id = Attributes["snapshotId"].toInt();
		delete d->scriptObjectSnapshots.take(snap_id);
	}
	else if(typeStr == "GetPropertiesByIterator") // used only in console commands
	{
		int iter_id = Attributes["iteratorId"].toInt();
		SV4ValueIterator *iter = d->scriptValueIterators.value(iter_id);
		Q_ASSERT(iter != 0);
		if (!iter) {
			Response["error"] = "InvalidArgumentIndex";
			return Response;
		}

		QVariantList Result;
		for(;iter->index < iter->snapshot.properties.size(); iter->index++)
			Result.append(iter->snapshot.properties[iter->index].toVariant());
		Response["result"] = Result;
		Response["type"] = "QScriptDebuggerValuePropertyList";
	}
	else if(typeStr == "DeleteScriptValueIterator") // used only in console commands
	{
		int iter_id = Attributes["iteratorId"].toInt();
		delete d->scriptValueIterators.take(iter_id);
	}
	
	else if (typeStr == "SetScriptValueProperty")
	{
		QVariantMap value = Attributes["scriptValue"].toMap();
		Q_ASSERT(value["type"] == "ObjectValue");
		UV4Handle Handle = { value["value"].toULongLong() };

		SV4Value Value;
		Value.fromVariant(Attributes["subordinateScriptValue"].toMap());

		CV4SetValueJob job(d->handler, Handle, Attributes["name"].toString(), Value);
		d->debugger->runJobInEngine(&job);
	}

	else if (typeStr == "ClearExceptions") // used only in console commands
	{
		d->debugger->engine()->hasException = false;
	}
		
	else // unknown commands
	{
		Q_ASSERT(0);
	}

	return Response;
}

void CV4ScriptDebuggerBackend::attachTo(class CV4EngineItf* engine)
{
	Q_D(CV4ScriptDebuggerBackend);

	d->engine = engine;
	d->debugger = new CV4DebugAgent(engine->self()->handle());
	d->handler = new CV4DebugHandler(engine->self()->handle(), this);
	connect(d->debugger, SIGNAL(debuggerPaused(CV4DebugAgent*, int, const QString&, int)), this, SLOT(debuggerPaused(CV4DebugAgent*, int, const QString&, int)));
	connect(d->engine->self(), SIGNAL(evaluateFinished(const QJSValue&)), this, SLOT(evaluateFinished(const QJSValue&)));
	connect(d->engine->self(), SIGNAL(printTrace(const QString&)), this, SLOT(printTrace(const QString&)));
	connect(d->engine->self(), SIGNAL(invokeDebugger()), this, SLOT(invokeDebugger()), Qt::BlockingQueuedConnection);
	d->debugger->setBreakOnException();
}

void CV4ScriptDebuggerBackend::pause()
{
	Q_D(CV4ScriptDebuggerBackend);

	d->debugger->pause();
}

void CV4ScriptDebuggerBackend::detach()
{
	Q_D(CV4ScriptDebuggerBackend);

	if (!d->debugger)
		return;

	d->debugger->resume(); // clear stepping
	d->debugger->setBreakOnException(false); // clear break on exception
	d->debugger->deleteAllBreakpoints(); // clear breakpoints
	d->debugger->resume(); // final resume
	d->debugger = NULL; // the engine will dispose of the debugger

	delete d->handler;
	d->handler = NULL;

	disconnect(this);
	d->engine = NULL;
}

void CV4ScriptDebuggerBackend::debuggerPaused(CV4DebugAgent* debugger, int reason, const QString& fileName, int lineNumber)
{
	Q_D(CV4ScriptDebuggerBackend);

	Q_ASSERT(debugger == d->debugger);

	int line = 0;
	if(debugger->engine()->currentStackFrame)
		line = debugger->engine()->currentStackFrame->lineNumber();

	QVariantMap Event;
	switch (reason)
	{
	case CV4DebugAgent::PauseRequest:	Event["type"] = "Interrupted"; break;
	case CV4DebugAgent::BreakPointHit:	Event["type"] = "Breakpoint"; break;
	case CV4DebugAgent::Stepped:		Event["type"] = "SteppingFinished"; break; 
	case CV4DebugAgent::LocationReached:Event["type"] = "LocationReached"; break;
	case CV4DebugAgent::DebuggerInvoked:Event["type"] = "DebuggerInvocationRequest"; break;
	case CV4DebugAgent::Exception:		Event["type"] = "Exception"; break;
	}
	QVariantMap Attributes;
	Attributes["scriptId"] = d->engine->getScriptId(QUrl(fileName).fileName());
	Attributes["fileName"] = QUrl(fileName).fileName();
	Attributes["lineNumber"] = lineNumber;
	Attributes["columnNumber"] = 0; // todo
	if (reason == CV4DebugAgent::Exception) {
		Attributes["message"] = d->engine->self()->handle()->exceptionValue->toQStringNoThrow();
		Attributes["value"] = d->engine->self()->toScriptValue(d->engine->self()->handle()->exceptionValue).toVariant();
		Attributes["hasExceptionHandler"] = false; // todo
	}
	Event["attributes"] = Attributes;

	d->pendingEvents.append(Event);
}

void CV4ScriptDebuggerBackend::evaluateFinished(const QJSValue& ret)
{
	QString Message;
	if (ret.isError()) {
		Message = tr("Uncaught exception in %1, at line %2: %3").arg(QUrl(ret.property("fileName").toString()).fileName())
			.arg(ret.property("lineNumber").toInt()).arg(ret.toString());
	} else
		Message = ret.toString();

	evalFinished(ret.toVariant(), Message);
}

void CV4ScriptDebuggerBackend::evalFinished(const QVariant& Value, const QString& Message)
{
	Q_D(CV4ScriptDebuggerBackend);

	QVariantMap Event;
	Event["type"] = "InlineEvalFinished";
	QVariantMap Attributes;
	Attributes["value"] = Value;
	Attributes["isNestedEvaluate"] = true; // = d->debugger->isPaused(); // then this is false, the gui will issue a resume isntruction
	Attributes["message"] = Message;
	Event["attributes"] = Attributes;

	d->pendingEvents.append(Event);
}

void CV4ScriptDebuggerBackend::printTrace(const QString& Message)
{
	Q_D(CV4ScriptDebuggerBackend);

	QVariantMap Event;
	Event["type"] = "Trace";
	QVariantMap Attributes;
	Attributes["message"] = Message;
	Event["attributes"] = Attributes;

	d->pendingEvents.append(Event);
}

void CV4ScriptDebuggerBackend::invokeDebugger()
{
	Q_D(CV4ScriptDebuggerBackend);

	if(d->debugger)
		d->debugger->pause(CV4DebugAgent::DebuggerInvoked);
}

QVariantMap CV4ScriptDebuggerBackend::scriptDelta()
{
	Q_D(CV4ScriptDebuggerBackend);

	QSet<qint64> prevSet = d->previousCheckpointScripts;
	QSet<qint64> currSet = d->checkpointScripts;
	QSet<qint64> addedScriptIds_set = (currSet - prevSet);
	QList<qint64> addedScriptIds = QList<qint64>(addedScriptIds_set.begin(), addedScriptIds_set.end());
	QSet<qint64> removedScriptIds_set = (prevSet - currSet);
	QList<qint64> removedScriptIds = QList<qint64>(removedScriptIds_set.begin(), removedScriptIds_set.end());

	QVariantList added;
	for (int i = 0; i < addedScriptIds.size(); ++i)
		added.append(addedScriptIds.at(i));
	QVariantList removed;
	for (int i = 0; i < removedScriptIds.size(); ++i)
		removed.append(removedScriptIds.at(i));
	QVariantMap result;
	result["added"] = added;
	result["removed"] = removed;

	return result;
}

