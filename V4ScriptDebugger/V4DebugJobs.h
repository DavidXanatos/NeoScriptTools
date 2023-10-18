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

#ifndef CV4DEBUGJOBS_H
#define CV4DEBUGJOBS_H

#include "V4DebugHandler.h"

////////////////////////////////////////////////////////////////////////////////////
// CV4DebugJob
//

class CV4DebugJob
{
public:
    virtual ~CV4DebugJob() {}

    virtual void run() = 0;
};

////////////////////////////////////////////////////////////////////////////////////
// CV4GetPropsJob
//

class CV4GetPropsJob : public CV4DebugJob
{
    class CV4DebugHandler* handler;
    UV4Handle handle;
    bool success;
    SV4Object result;

public:
    CV4GetPropsJob(CV4DebugHandler* handler, UV4Handle handle);
    void run() override;

    bool wasSuccessful() const { return success; }
    const SV4Object& returnValue() const { return result; }
};

////////////////////////////////////////////////////////////////////////////////////
// CV4SetValueJob
//

class CV4SetValueJob : public CV4DebugJob
{
    class CV4DebugHandler* handler;
    UV4Handle handle;
    QString name;
    SV4Value value;
    bool success;

public:
    CV4SetValueJob(CV4DebugHandler* handler, UV4Handle handle, const QString& name, const SV4Value& value);
    void run() override;

    bool wasSuccessful() const { return success; }
};

////////////////////////////////////////////////////////////////////////////////////
// CV4ScriptJob
//

class CV4ScriptJob : public CV4DebugJob
{
    QV4::ExecutionEngine* engine;
    int frameNr;
    //int context;
    const QString& program;
    bool resultIsException;

public:
    CV4ScriptJob(QV4::ExecutionEngine* engine, const QString& program, int frameNr/*, int context*/);

    void run() override;

    static QV4::ScopedValue exec(QV4::ExecutionEngine* engine, QV4::Scope& scope, const QString& program, int frameNr/*, int context*/);

    bool hasExeption() const { return resultIsException; }

protected:
    virtual void handleResult(QV4::ScopedValue& result) = 0;
};

////////////////////////////////////////////////////////////////////////////////////
// CV4RunScriptJob
//

class CV4RunScriptJob : public CV4ScriptJob
{
    class CV4DebugHandler* handler;
    QString exception;
    SV4Object result;

public:
    CV4RunScriptJob(QV4::ExecutionEngine* engine, CV4DebugHandler* handler, const QString& program, int frameNr/*, int context*/);
    void handleResult(QV4::ScopedValue& value) override;

    const QString& exceptionMessage() const { return exception; }
    const SV4Object& returnValue() const { return result; }
};

#endif