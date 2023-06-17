/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSCriptTools module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSCRIPTDEBUGGERCODEVIEWINTERFACE_P_H
#define QSCRIPTDEBUGGERCODEVIEWINTERFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#if QT_VERSION < 0x050000
#include <QtGui/qwidget.h>
#else
#include <QtWidgets/qwidget.h>
#endif

QT_BEGIN_NAMESPACE

class QPoint;

class QScriptDebuggerCodeViewInterfacePrivate;
class Q_AUTOTEST_EXPORT QScriptDebuggerCodeViewInterface:
    public QWidget
{
    Q_OBJECT
public:
    ~QScriptDebuggerCodeViewInterface();

	virtual QString name() const = 0;
    virtual void setName(const QString &name) = 0;

    virtual QString text() const = 0;
    virtual void setText(const QString &text) = 0;

    virtual int cursorLineNumber() const = 0;
    virtual void gotoLine(int lineNumber) = 0;

    virtual int find(const QString &exp, int options = 0) = 0;

    virtual void setExecutionLineNumber(int lineNumber, bool error) = 0;
    //virtual void setExecutableLineNumbers(const QSet<int> &lineNumbers) = 0;

    virtual int baseLineNumber() const = 0;
    virtual void setBaseLineNumber(int lineNumber) = 0;

    virtual void setBreakpoint(int lineNumber) = 0;
    virtual void deleteBreakpoint(int lineNumber) = 0;
    virtual void setBreakpointEnabled(int lineNumber, bool enable) = 0;

	//> NeoScriptTools
	virtual void setReadOnly(bool set) = 0;

	virtual void setModified(bool set) = 0;
	//< NeoScriptTools

Q_SIGNALS:
    void breakpointToggleRequest(int lineNumber, bool on);
    void breakpointEnableRequest(int lineNumber, bool enable);
    void toolTipRequest(const QPoint &pos, int lineNumber, const QStringList &path);
	//> NeoScriptTools
	void modificationChanged (bool changed);
	//< NeoScriptTools

protected:
    QScriptDebuggerCodeViewInterface(
        QScriptDebuggerCodeViewInterfacePrivate &dd,
        QWidget *parent, Qt::WindowFlags flags);

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerCodeViewInterface)
    Q_DISABLE_COPY(QScriptDebuggerCodeViewInterface)
};

QT_END_NAMESPACE

#endif
