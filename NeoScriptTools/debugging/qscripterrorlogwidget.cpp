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

#include "qscripterrorlogwidget_p.h"
#include "qscripterrorlogwidgetinterface_p_p.h"

#include <QtCore/qdatetime.h>
#if QT_VERSION < 0x050000
#include <QtGui/qboxlayout.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qscrollbar.h>
#else
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qscrollbar.h>
#endif
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace {

class QScriptErrorLogWidgetOutputEdit : public QTextEdit
{
public:
    QScriptErrorLogWidgetOutputEdit(QWidget *parent = 0)
        : QTextEdit(parent)
    {
        setReadOnly(true);
//        setFocusPolicy(Qt::NoFocus);
        document()->setMaximumBlockCount(255);
    }

    void scrollToBottom()
    {
        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
    }
};

} // namespace

class QScriptErrorLogWidgetPrivate
    : public QScriptErrorLogWidgetInterfacePrivate
{
    Q_DECLARE_PUBLIC(QScriptErrorLogWidget)
public:
    QScriptErrorLogWidgetPrivate();
    ~QScriptErrorLogWidgetPrivate();

    QScriptErrorLogWidgetOutputEdit *outputEdit;
};

QScriptErrorLogWidgetPrivate::QScriptErrorLogWidgetPrivate()
{
}

QScriptErrorLogWidgetPrivate::~QScriptErrorLogWidgetPrivate()
{
}

QScriptErrorLogWidget::QScriptErrorLogWidget(QWidget *parent)
    : QScriptErrorLogWidgetInterface(*new QScriptErrorLogWidgetPrivate, parent, {})
{
    Q_D(QScriptErrorLogWidget);
    d->outputEdit = new QScriptErrorLogWidgetOutputEdit();
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(d->outputEdit);

//    QString sheet = QString::fromLatin1("font-size: 14px; font-family: \"Monospace\";");
//    setStyleSheet(sheet);
}

QScriptErrorLogWidget::~QScriptErrorLogWidget()
{
}

void QScriptErrorLogWidget::message(
    QtMsgType type, const QString &text, const QString &fileName,
    int lineNumber, int columnNumber, const QVariant &/*data*/)
{
    // ### we need the error message rather than Error.toString()
    Q_UNUSED(type);
    Q_UNUSED(fileName);
    Q_UNUSED(lineNumber);
    Q_UNUSED(columnNumber);
    Q_D(QScriptErrorLogWidget);
    QString html;
#if QT_VERSION < 0x050000
    html.append(QString::fromLatin1("<b>%0</b> %1<br>")
                .arg(QDateTime::currentDateTime().toString()).arg(Qt::escape(text)));
#else
    html.append(QString::fromLatin1("<b>%0</b> %1<br>")
                .arg(QDateTime::currentDateTime().toString()).arg(text.toHtmlEscaped()));
#endif
    d->outputEdit->insertHtml(html);
    d->outputEdit->scrollToBottom();
}

void QScriptErrorLogWidget::clear()
{
    Q_D(QScriptErrorLogWidget);
    d->outputEdit->clear();
}

QT_END_NAMESPACE
