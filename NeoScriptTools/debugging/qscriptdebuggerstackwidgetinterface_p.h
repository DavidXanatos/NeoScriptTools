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

#ifndef QSCRIPTDEBUGGERSTACKWIDGETINTERFACE_P_H
#define QSCRIPTDEBUGGERSTACKWIDGETINTERFACE_P_H

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

class QAbstractItemModel;

class QScriptDebuggerStackWidgetInterfacePrivate;
class Q_AUTOTEST_EXPORT QScriptDebuggerStackWidgetInterface:
    public QWidget
{
    Q_OBJECT
public:
    ~QScriptDebuggerStackWidgetInterface();

    virtual QAbstractItemModel *stackModel() const = 0;
    virtual void setStackModel(QAbstractItemModel *model) = 0;

    virtual int currentFrameIndex() const = 0;
    virtual void setCurrentFrameIndex(int frameIndex) = 0;

Q_SIGNALS:
    void currentFrameChanged(int newFrameIndex);

protected:
    QScriptDebuggerStackWidgetInterface(
        QScriptDebuggerStackWidgetInterfacePrivate &dd,
        QWidget *parent, Qt::WindowFlags flags);

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerStackWidgetInterface)
    Q_DISABLE_COPY(QScriptDebuggerStackWidgetInterface)
};

QT_END_NAMESPACE

#endif
