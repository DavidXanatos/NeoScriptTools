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

#include "qscriptstdmessagehandler_p.h"
#include <stdio.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptStdMessageHandler
  \internal

  \brief The QScriptStdMessageHandler class implements a message handler that writes to standard output.
*/

class QScriptStdMessageHandlerPrivate
{
public:
    QScriptStdMessageHandlerPrivate() {}
    ~QScriptStdMessageHandlerPrivate() {}
};

QScriptStdMessageHandler::QScriptStdMessageHandler()
    : d_ptr(new QScriptStdMessageHandlerPrivate)
{
}

QScriptStdMessageHandler::~QScriptStdMessageHandler()
{
}

void QScriptStdMessageHandler::message(QtMsgType type, const QString &text,
                                       const QString &fileName,
                                       int lineNumber, int columnNumber,
                                       const QVariant &/*data*/)
{
    QString msg;
    if (!fileName.isEmpty() || (lineNumber != -1)) {
        if (!fileName.isEmpty())
            msg.append(fileName);
        else
            msg.append(QLatin1String("<noname>"));
        if (lineNumber != -1) {
            msg.append(QLatin1Char(':'));
            msg.append(QString::number(lineNumber));
            if (columnNumber != -1) {
                msg.append(QLatin1Char(':'));
                msg.append(QString::number(columnNumber));
            }
        }
        msg.append(QLatin1String(": "));
    }
    msg.append(text);

    FILE *fp = (type == QtDebugMsg) ? stdout : stderr;
    fprintf(fp, "%s\n", msg.toLatin1().constData());
    fflush(fp);
}

QT_END_NAMESPACE
