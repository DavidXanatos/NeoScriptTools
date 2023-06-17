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

#ifndef QSCRIPTDEBUGGERLOCALSMODEL_P_H
#define QSCRIPTDEBUGGERLOCALSMODEL_P_H

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

#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerValue;
class QScriptDebuggerJobSchedulerInterface;
class QScriptDebuggerCommandSchedulerInterface;

class QScriptDebuggerLocalsModelPrivate;
class Q_AUTOTEST_EXPORT QScriptDebuggerLocalsModel
    : public QAbstractItemModel
{
    Q_OBJECT
public:
    QScriptDebuggerLocalsModel(QScriptDebuggerJobSchedulerInterface *jobScheduler,
                               QScriptDebuggerCommandSchedulerInterface *commandScheduler,
                               QObject *parent = 0);
    ~QScriptDebuggerLocalsModel();

    void init(int frameIndex);
    void sync(int frameIndex);

    int frameIndex() const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool hasChildren(const QModelIndex &parent) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

Q_SIGNALS:
    void scopeObjectAvailable(const QModelIndex &index);

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerLocalsModel)
    Q_DISABLE_COPY(QScriptDebuggerLocalsModel)
};

QT_END_NAMESPACE

#endif
