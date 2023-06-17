/****************************************************************************
**
** Copyright (C) 2012 NeoLoader Team
** All rights reserved.
** Contact: XanatosDavid@gmil.com
**
** This file is part of the V8ScriptTools module for NeoLoader
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

#ifndef QSCRIPTDEBUGGERCONTEXTINFO_P_H
#define QSCRIPTDEBUGGERCONTEXTINFO_P_H

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

#include <QtCore/qobjectdefs.h>
#include <QtCore/private/qscopedpointer_p.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerContextInfoPrivate;
class Q_AUTOTEST_EXPORT QScriptDebuggerContextInfo
{
public:
    enum FunctionType {
        ScriptFunction,
        QtFunction,
        QtPropertyFunction,
        NativeFunction
    };

    QScriptDebuggerContextInfo();
    QScriptDebuggerContextInfo(qint64 scriptId, const QString& fileName, int lineNumber, int columnNumber, const QString& functionName, FunctionType functionType);
    QScriptDebuggerContextInfo(const QScriptDebuggerContextInfo &other);
    ~QScriptDebuggerContextInfo();

    void fromVariant(const QVariantMap& in);

    bool isNull() const;

    qint64 scriptId() const;
    QString fileName() const;
    int lineNumber() const;
	int columnNumber() const;

    QString functionName() const;
    FunctionType functionType() const;

	QScriptDebuggerContextInfo &operator=(const QScriptDebuggerContextInfo &other);

private:
	QScopedSharedPointer<QScriptDebuggerContextInfoPrivate> d_ptr;

	Q_DECLARE_PRIVATE(QScriptDebuggerContextInfo)
};

QT_END_NAMESPACE

#endif


