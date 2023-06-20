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

#include "V4DebugAgent.h"
#include <QThread>

#include <private/qv4script_p.h>

#include "V4DebugJobs.h"

inline uint qHash(const CV4DebugAgent::SBreakKey& v, uint seed = 0)
{
	return v.lineNumber ^ qHash(v.fileName, seed);
}

inline bool operator==(const CV4DebugAgent::SBreakKey& l, const CV4DebugAgent::SBreakKey& r)
{
	return l.lineNumber == r.lineNumber && l.fileName == r.fileName;
}

void SV4Breakpoint::fromVariant(const QVariantMap& in)
{
	fileName = in["fileName"].toString();
	lineNumber = in["lineNumber"].toInt();
	enabled = in["enabled"].toBool();
	singleShot = in["singleShot"].toBool();
	ignoreCount = in["ignoreCount"].toInt();
	condition = in["condition"].toString();
	data = in["data"];
	hitCount = in["hitCount"].toInt();
}

QVariantMap SV4Breakpoint::toVariant() const
{
	QVariantMap out;
	out["fileName"] = fileName;
	out["lineNumber"] = lineNumber;
	out["enabled"] = enabled;
	out["singleShot"] = singleShot;
	out["ignoreCount"] = ignoreCount;
	out["condition"] = condition;
	out["data"] = data;
	out["hitCount"] = hitCount;
	return out;
}


////////////////////////////////////////////////////////////////////////////////////
// CV4DebugAgent
//

CV4DebugAgent::CV4DebugAgent(QV4::ExecutionEngine* engine) 
{
	m_engine = engine;
	m_breakOnException = false;
	m_pauseRequested = DontBreak;
	m_paused = false;
	m_currentFrame = nullptr;
	m_steppingMode = NotStepping;
	m_breakpointIdCtr = 0;
	m_haveBreakpoints = 0;
	m_runningJob = nullptr;

	m_engine->setDebugger(this);
}

void CV4DebugAgent::pause(PauseReason reason)
{
	QMutexLocker locker(&m_mutex);
	if (m_paused)
		return;
	m_pauseRequested = reason;
}

void CV4DebugAgent::resume(Stepping stepping)
{
	QMutexLocker locker(&m_mutex);
	if (!m_paused)
		return;

	m_currentFrame = m_engine->currentStackFrame;
	m_steppingMode = stepping;
	m_engineWaiter.wakeAll();
}

void CV4DebugAgent::runJobInEngine(class CV4DebugJob* job, bool bWait)
{
	QMutexLocker locker(&m_mutex);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Note: this must be called from a different thread than the engine and agent are executed in
	// the user of this agent must ensure that it always lives in the same thread as the engine.
	Q_ASSERT(QThread::currentThread() != QObject::thread());

	m_runningJob = job; // schedule job
	if (m_paused) // resume engine when paused, signalAndWait will run the job
		m_engineWaiter.wakeAll(); 
	else // invoke function in the engines thread
		QMetaObject::invokeMethod(this, "runJob", Qt::QueuedConnection);
		/*QMetaObject::invokeMethod(this, [&](){
				runJob(); // or like this
			}, Qt::QueuedConnection);*/
		
	if (bWait)
		m_jobWaiter.wait(&m_mutex);
}

void CV4DebugAgent::runJob()
{
	QMutexLocker locker(&m_mutex);

	if (m_runningJob) {
		//m_mutex.unlock();
		m_runningJob->run();
		//m_mutex.lock();
	}
	m_jobWaiter.wakeAll();
	m_runningJob = nullptr;
}

void CV4DebugAgent::runUntil(const QString& fileName, int lineNumber)
{
	QMutexLocker locker(&m_mutex);

	clearRunUntil();
	// set a dummy breakpoint
	m_breakpointHash.insert(SBreakKey(fileName, lineNumber), (SV4Breakpoint*)-1);
}

void CV4DebugAgent::clearRunUntil()
{
	auto keys = m_breakpointHash.keys((SV4Breakpoint*)-1);
	while (!keys.isEmpty())
		m_breakpointHash.remove(keys.takeFirst());
}

QV4::CppStackFrame* CV4DebugAgent::findFrame(QV4::ExecutionEngine* engine, int frameNr)
{
	QV4::CppStackFrame* frame = engine->currentStackFrame;
	for (int i = 0; frame && i < frameNr; i++)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		frame = frame->parent;
#else
		frame = frame->parentFrame();
#endif
	return frame;
}

QV4::Heap::ExecutionContext* CV4DebugAgent::findContext(QV4::ExecutionEngine* engine, int frameNr)
{
	QV4::CppStackFrame* frame = findFrame(engine, frameNr);
	return frame ? frame->context()->d() : nullptr;
}

QV4::Heap::ExecutionContext* CV4DebugAgent::findScope(QV4::Heap::ExecutionContext* ctx, int scopeNr)
{
	for (int i = 0; ctx && i < scopeNr; i++)
		ctx = ctx->outer;
	return ctx;
}

QVector<SV4Scope> CV4DebugAgent::getScopes(int frame)
{
	QVector<SV4Scope> scopes;
	if (!m_paused) // engine must be paused as we access it here
		return scopes;
	
	QV4::Heap::ExecutionContext* ec = findFrame(m_engine, frame)->context()->d();
	for (int i = 0; ec; ec = ec->outer, i++) {
		QString type;
		switch (ec->type) {
		default:												type = "Unknown"; break;
		case QV4::Heap::ExecutionContext::Type_GlobalContext:	type = "GlobalContext"; break;
		case QV4::Heap::ExecutionContext::Type_WithContext:		type = "WithContext"; break;
		case QV4::Heap::ExecutionContext::Type_QmlContext:		type = "QmlContext"; break;
		case QV4::Heap::ExecutionContext::Type_BlockContext:	type = "BlockContext"; break;
		case QV4::Heap::ExecutionContext::Type_CallContext:		type = "CallContext"; break;
		}
		scopes.append(SV4Scope{ i, type });
	}
	return scopes;
}

int CV4DebugAgent::setBreakpoint(const SV4Breakpoint& Breakpoint)
{
	QMutexLocker locker(&m_mutex);

	int id = ++m_breakpointIdCtr;

	SV4Breakpoint& bp = m_breakpoints[id];
	bp = Breakpoint;
	m_breakpointHash.insert(SBreakKey(bp.fileName, bp.lineNumber), &bp);
	m_haveBreakpoints = true;

	return id;
}

void CV4DebugAgent::deleteBreakpoint(int id)
{
	QMutexLocker locker(&m_mutex);

	SV4Breakpoint bp = m_breakpoints.take(id);
	m_breakpointHash.remove(SBreakKey(bp.fileName, bp.lineNumber));
	m_haveBreakpoints = !m_breakpoints.isEmpty();
}

void CV4DebugAgent::deleteAllBreakpoints()
{
	QMutexLocker locker(&m_mutex);

	m_breakpoints.clear();
	m_breakpointHash.clear();
	m_haveBreakpoints = false;
}

bool CV4DebugAgent::updateBreakpoint(int id, const SV4Breakpoint& Breakpoint)
{
	QMutexLocker locker(&m_mutex);

	auto I = m_breakpoints.find(id);
	if (I == m_breakpoints.end())
		return false;
	m_breakpointHash.remove(SBreakKey(I->fileName, I->lineNumber));
	*I = Breakpoint;
	m_breakpointHash.insert(SBreakKey(I->fileName, I->lineNumber), &*I);
	return true;
}

CV4DebugAgent::PauseReason CV4DebugAgent::checkBreakpoints(const QString& fileName, int lineNumber)
{
	auto I = m_breakpointHash.find(SBreakKey(QUrl(fileName).fileName(), lineNumber));
	if (I == m_breakpointHash.end())
		return DontBreak;

	SV4Breakpoint* bp = *I;
	if (bp == (SV4Breakpoint*)-1)
		return LocationReached;
	if (!bp->enabled)
		return DontBreak;

	if (!bp->condition.isEmpty()) {
		Q_ASSERT(m_runningJob == nullptr);

		m_runningJob = (CV4DebugJob*)-1; // set dumy job to not enter maybeBreakAtInstruction
		QV4::Scope scope(m_engine);
		QV4::ScopedValue result = CV4ScriptJob::exec(m_engine, scope, bp->condition, -1/*, -1*/);
		m_runningJob = nullptr;
		if (!result->toBoolean())
			return DontBreak;
	}

	if (bp->ignoreCount > 0) {
		bp->ignoreCount--;
		return DontBreak;
	}

	if (bp->singleShot)
		bp->enabled = false;

	bp->hitCount++;
	return BreakPointHit;
}

void CV4DebugAgent::signalAndWait(PauseReason reason)
{
	if (m_runningJob)
		return;
	m_paused = true;

	// cleanup dummy breakpoints
	clearRunUntil();

	// capture stack trace for this break
	// saves on computation if the caller accesses the trace one frame at a time
	m_stackTrace = m_engine->stackTrace();

	// notify the debugger
	if(m_engine->currentStackFrame)
		emit debuggerPaused(this, reason, m_engine->currentStackFrame->v4Function->sourceFile(), m_engine->currentStackFrame->lineNumber());
	else if(m_engine->globalCode)
		emit debuggerPaused(this, reason, m_engine->globalCode->sourceFile(), 1);
	else
		emit debuggerPaused(this, reason, QStringLiteral("unknown"), 1);

	// wait and run jobs
	for (;;) {
		m_engineWaiter.wait(&m_mutex);

		if (!m_runningJob)
			break;
		
		//m_mutex.unlock();
		m_runningJob->run();
		//m_mutex.lock();

		m_jobWaiter.wakeAll();
		m_runningJob = nullptr;
	}

	m_paused = false;
}

////////////////////////////////////////////////////////////////////////////////////
// QV4::Debugging::Debugger
//

bool CV4DebugAgent::pauseAtNextOpportunity() const
{
	return m_pauseRequested
		|| m_haveBreakpoints
		|| m_steppingMode >= StepOver;
}

void CV4DebugAgent::maybeBreakAtInstruction()
{
	if (m_runningJob) // keep running when in job
		return;

	QMutexLocker locker(&m_mutex);

	switch (m_steppingMode) {
	case StepOver:
		if (m_currentFrame != m_engine->currentStackFrame)
			break;
	case StepIn:
		signalAndWait(Stepped);
		return;
	case StepOut:
	case NotStepping:
		break;
	}

	PauseReason pause = DontBreak;
	if (m_pauseRequested) {
		pause = m_pauseRequested;
		m_pauseRequested = DontBreak;
	}
	else if (m_haveBreakpoints)
		pause = checkBreakpoints(m_engine->currentStackFrame->v4Function->sourceFile(), m_engine->currentStackFrame->lineNumber());
	if (pause != DontBreak)
		signalAndWait(pause);
}

void CV4DebugAgent::enteringFunction()
{
	if (m_runningJob)
		return;
	QMutexLocker locker(&m_mutex);

	QString fileName = QUrl(m_engine->currentStackFrame->v4Function->sourceFile()).fileName();
	m_scriptIdStack.append(fileName);

	if (m_steppingMode == StepIn)
		m_currentFrame = m_engine->currentStackFrame;
}

void CV4DebugAgent::leavingFunction(const QV4::ReturnedValue& retVal)
{
	if (m_runningJob)
		return;
	QMutexLocker locker(&m_mutex);

	m_scriptIdStack.removeLast();

	if (m_steppingMode != NotStepping && m_currentFrame == m_engine->currentStackFrame) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_currentFrame = m_currentFrame->parent;
#else
		m_currentFrame = m_currentFrame->parentFrame();
#endif
		m_steppingMode = StepOver;
	}
}

void CV4DebugAgent::aboutToThrow()
{
	if (!m_breakOnException)
		return;

	if (m_runningJob) // ignore exceptions in jobs
		return;

	QMutexLocker locker(&m_mutex);
	signalAndWait(Exception);
}