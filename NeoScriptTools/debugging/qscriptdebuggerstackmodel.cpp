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

#include "qscriptdebuggerstackmodel_p.h"

#include "private/qabstractitemmodel_p.h"

#include "qscriptdebuggercontextinfo_p.h"
#include <QtCore/qfileinfo.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerStackModelPrivate
    : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QScriptDebuggerStackModel)
public:
    QScriptDebuggerStackModelPrivate();
    ~QScriptDebuggerStackModelPrivate();

    QList<QScriptDebuggerContextInfo> contextInfos;
};

QScriptDebuggerStackModelPrivate::QScriptDebuggerStackModelPrivate()
{
}

QScriptDebuggerStackModelPrivate::~QScriptDebuggerStackModelPrivate()
{
}

QScriptDebuggerStackModel::QScriptDebuggerStackModel(QObject *parent)
    : QAbstractTableModel(*new QScriptDebuggerStackModelPrivate, parent)
{
}

QScriptDebuggerStackModel::~QScriptDebuggerStackModel()
{
}

QList<QScriptDebuggerContextInfo> QScriptDebuggerStackModel::contextInfos() const
{
    Q_D(const QScriptDebuggerStackModel);
    return d->contextInfos;
}

void QScriptDebuggerStackModel::setContextInfos(const QList<QScriptDebuggerContextInfo> &infos)
{
    Q_D(QScriptDebuggerStackModel);
    layoutAboutToBeChanged();
    d->contextInfos = infos;
    layoutChanged();
}

/*!
  \reimp
*/
int QScriptDebuggerStackModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 3;
    return 0;
}

/*!
  \reimp
*/
int QScriptDebuggerStackModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QScriptDebuggerStackModel);
    if (!parent.isValid())
        return d->contextInfos.count();
    return 0;
}

/*!
  \reimp
*/
QVariant QScriptDebuggerStackModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QScriptDebuggerStackModel);
    if (!index.isValid())
        return QVariant();
    if (index.row() >= d->contextInfos.count())
        return QVariant();
    const QScriptDebuggerContextInfo &info = d->contextInfos.at(index.row());
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return index.row();
        } else if (index.column() == 1) {
            QString name = info.functionName();
            if (name.isEmpty())
                name = QString::fromLatin1("<anonymous>");
            return name;
        } else if (index.column() == 2) {
            QString fn = QFileInfo(info.fileName()).fileName();
            if (fn.isEmpty()) {
                if (info.functionType() == QScriptDebuggerContextInfo::ScriptFunction)
                    fn = QString::fromLatin1("<anonymous script, id=%0>").arg(info.scriptId());
                else
                    fn = QString::fromLatin1("<native>");

            }
            return QString::fromLatin1("%0:%1").arg(fn).arg(info.lineNumber());
        }
    } else if (role == Qt::ToolTipRole) {
        if (QFileInfo(info.fileName()).fileName() != info.fileName())
            return info.fileName();
    }
    return QVariant();
}

/*!
  \reimp
*/
QVariant QScriptDebuggerStackModel::headerData(int section, Qt::Orientation orient, int role) const
{
    if (orient != Qt::Horizontal)
        return QVariant();
    if (role == Qt::DisplayRole) {
        if (section == 0)
            return QCoreApplication::translate("QScriptDebuggerStackModel", "Level");
        else if (section == 1)
            return QCoreApplication::translate("QScriptDebuggerStackModel", "Name");
        else if (section == 2)
            return QCoreApplication::translate("QScriptDebuggerStackModel", "Location");
    }
    return QVariant();
}

QT_END_NAMESPACE
