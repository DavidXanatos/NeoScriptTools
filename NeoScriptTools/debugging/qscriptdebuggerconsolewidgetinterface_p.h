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

#ifndef QSCRIPTDEBUGGERCONSOLEWIDGETINTERFACE_P_H
#define QSCRIPTDEBUGGERCONSOLEWIDGETINTERFACE_P_H

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

#include "qscriptmessagehandlerinterface_p.h"

QT_BEGIN_NAMESPACE

class QScriptDebuggerConsoleHistorianInterface;
class QScriptCompletionProviderInterface;

class QScriptDebuggerConsoleWidgetInterfacePrivate;
class Q_AUTOTEST_EXPORT QScriptDebuggerConsoleWidgetInterface:
    public QWidget,
    public QScriptMessageHandlerInterface
{
    Q_OBJECT
public:
    enum InputMode {
        NormalInputMode,
        PartialInputMode
    };

    ~QScriptDebuggerConsoleWidgetInterface();

    QScriptDebuggerConsoleHistorianInterface *commandHistorian() const;
    void setCommandHistorian(QScriptDebuggerConsoleHistorianInterface *historian);

    QScriptCompletionProviderInterface *completionProvider() const;
    void setCompletionProvider(QScriptCompletionProviderInterface *completionProvider);

    virtual void setLineContinuationMode(bool enabled) = 0;

    virtual void clear() = 0;

Q_SIGNALS:
    void lineEntered(const QString &contents);

protected:
    QScriptDebuggerConsoleWidgetInterface(QScriptDebuggerConsoleWidgetInterfacePrivate &dd,
                                          QWidget *parent, Qt::WindowFlags flags);

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerConsoleWidgetInterface)
    Q_DISABLE_COPY(QScriptDebuggerConsoleWidgetInterface)
};

QT_END_NAMESPACE

#endif
