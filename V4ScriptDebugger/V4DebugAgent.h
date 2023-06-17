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

#ifndef CV4DEBUGAGENT_H
#define CV4DEBUGAGENT_H

#include <QObject>

#include <private/qv4engine_p.h>
#include <private/qv4debugging_p.h>
#include <private/qv4function_p.h>
#include <private/qv4context_p.h>
#include <private/qv4persistent_p.h>

#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>

class CV4DebugJob;

struct SV4Breakpoint {

    void fromVariant(const QVariantMap& in);
    QVariantMap toVariant() const;

    QString fileName;
    int lineNumber;
    bool enabled;
    bool singleShot;
    int ignoreCount;
    QString condition;
    QVariant data;
    int hitCount;
};

struct SV4Scope {
    int index;
    int type;
};

class CV4DebugAgent : public QV4::Debugging::Debugger
{
    Q_OBJECT

public:
    CV4DebugAgent(QV4::ExecutionEngine* engine, QObject* parent = nullptr);

    QV4::ExecutionEngine* engine() const { return m_engine; }

    enum Stepping {
        NotStepping = 0,
        StepOut,
        StepOver,
        StepIn,
    };

    enum PauseReason {
        DontBreak = 0,
        PauseRequest,
        BreakPointHit,
        DebuggerInvoked,
        Exception,
        Stepped,
        LocationReached
    };

    void pause(PauseReason reason = PauseRequest);
    bool isPaused() const { return m_paused; }
    void resume(Stepping stepping = NotStepping);
    void runUntil(const QString& fileName, int lineNumber);

    QVector<QV4::StackFrame> stackTrace() const { return m_stackTrace; }
    QVector<SV4Scope> getScopes(int frameNr);

    void runJobInEngine(class CV4DebugJob* job, bool bWait = true);

    void setBreakOnException(bool set = true) { m_breakOnException = set; }
    bool breakOnException() const { return m_breakOnException; }

    int setBreakpoint(const SV4Breakpoint& Breakpoint);
    QMap<int, SV4Breakpoint> getBreakpoints() const { QMutexLocker locker(&m_mutex); return m_breakpoints; }
    void deleteBreakpoint(int id);
    void deleteAllBreakpoints();
    bool updateBreakpoint(int id, const SV4Breakpoint& Breakpoint);

    struct SBreakKey {
        SBreakKey(const QString& fileName, int lineNumber)
            : fileName(fileName.mid(fileName.lastIndexOf('/') + 1)), lineNumber(lineNumber) {}
        QString fileName;
        int lineNumber;
    };

    QSet<QString> getCurrentScripts() const { QMutexLocker locker(&m_mutex); return QSet<QString>(m_scriptIdStack.begin(), m_scriptIdStack.end()); }

    static QV4::CppStackFrame* findFrame(QV4::ExecutionEngine* engine, int frameNr);
    static QV4::Heap::ExecutionContext* findContext(QV4::ExecutionEngine* engine, int frameNr);
    static QV4::Heap::ExecutionContext* findScope(QV4::Heap::ExecutionContext* ctx, int scopeNr);

signals:
    void debuggerPaused(CV4DebugAgent* self, int reason, const QString& fileName, int lineNumber);

private slots:
    void runJob();

protected:
    virtual bool pauseAtNextOpportunity() const override;
    virtual void maybeBreakAtInstruction() override;
    virtual void enteringFunction() override;
    virtual void leavingFunction(const QV4::ReturnedValue& retVal) override;
    virtual void aboutToThrow() override;

    PauseReason checkBreakpoints(const QString& fileName, int lineNumber);
    void clearRunUntil();
    void signalAndWait(PauseReason reason);

    QV4::ExecutionEngine* m_engine;
    bool m_breakOnException;
    PauseReason m_pauseRequested;
    bool m_paused;
    QV4::CppStackFrame* m_currentFrame;
    QVector<QV4::StackFrame> m_stackTrace;
    Stepping m_steppingMode;

    // breakpoints
    QHash<SBreakKey, SV4Breakpoint*> m_breakpointHash;
    QMap<int, SV4Breakpoint> m_breakpoints;
    int m_breakpointIdCtr;
    bool m_haveBreakpoints;

    // script tracking
    QList<QString> m_scriptIdStack;

    // synchronization and jobs
    mutable QMutex m_mutex;
    QWaitCondition m_engineWaiter; // holds the engine untill the debugger resumes
    QWaitCondition m_jobWaiter; // waits for the job to finish
    CV4DebugJob* m_runningJob;
};

#endif