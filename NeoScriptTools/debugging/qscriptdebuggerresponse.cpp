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

#include "qscriptdebuggerresponse_p.h"
#include "qscriptdebuggervalue_p.h"

#include <QtCore/qdatastream.h>

Q_DECLARE_METATYPE(QScriptBreakpointData)
Q_DECLARE_METATYPE(QScriptBreakpointMap)
Q_DECLARE_METATYPE(QScriptScriptData)
Q_DECLARE_METATYPE(QScriptScriptMap)
Q_DECLARE_METATYPE(QScriptDebuggerValue)
Q_DECLARE_METATYPE(QScriptDebuggerValueList)
Q_DECLARE_METATYPE(QScriptDebuggerValueProperty)
Q_DECLARE_METATYPE(QScriptDebuggerValuePropertyList)
//Q_DECLARE_METATYPE(QScriptContextInfo)

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptDebuggerResponse
  \internal

  \brief The QScriptDebuggerResponse class represents a front-end's response to a QScriptDebuggerCommand.

  A response contains an error code and result.
*/

class QScriptDebuggerResponsePrivate
{
public:
    QScriptDebuggerResponsePrivate();
    ~QScriptDebuggerResponsePrivate();

    QScriptDebuggerResponse::Error error;
    QVariant result;
    bool async;
	QString type;
};

QScriptDebuggerResponsePrivate::QScriptDebuggerResponsePrivate()
{
    error = QScriptDebuggerResponse::NoError;
    async = false;
}

QScriptDebuggerResponsePrivate::~QScriptDebuggerResponsePrivate()
{
}

QScriptDebuggerResponse::QScriptDebuggerResponse()
    : d_ptr(new QScriptDebuggerResponsePrivate)
{
}

QScriptDebuggerResponse::QScriptDebuggerResponse(const QScriptDebuggerResponse &other)
    : d_ptr(new QScriptDebuggerResponsePrivate)
{
    *d_ptr = *other.d_ptr;
}

QScriptDebuggerResponse::~QScriptDebuggerResponse()
{
}

QScriptDebuggerResponse &QScriptDebuggerResponse::operator=(const QScriptDebuggerResponse &other)
{
    *d_ptr = *other.d_ptr;
    return *this;
}

/*!
  Returns the error code of this response.
*/
QScriptDebuggerResponse::Error QScriptDebuggerResponse::error() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->error;
}

/*!
  Sets the \a error code of this response.
*/
void QScriptDebuggerResponse::setError(Error error)
{
    Q_D(QScriptDebuggerResponse);
    d->error = error;
}

/*!
  Returns the result of this response. This function is provided for
  convenience.
*/
QVariant QScriptDebuggerResponse::result() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->result;
}

/*!
  Returns the result type of this response. This function is provided for
  convenience.
*/
QString QScriptDebuggerResponse::type() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->type;
}

/*!
  Sets the Result attribute of this response to the given \a
  value. This function is provided for convenience.
*/
void QScriptDebuggerResponse::setResult(const QVariant &value, const QString& type)
{
    Q_D(QScriptDebuggerResponse);
    d->result = value;
	d->type = type;
}

void QScriptDebuggerResponse::setResult(int value)
{
    Q_D(QScriptDebuggerResponse);
    d->result = value;
	//d->type = "int";
}

void QScriptDebuggerResponse::setResult(const QString &value)
{
    Q_D(QScriptDebuggerResponse);
    d->result = value;
	//d->type = "QString";
}

void QScriptDebuggerResponse::setResult(const QScriptBreakpointData &data)
{
    Q_D(QScriptDebuggerResponse);
    //d->result = QVariant::fromValue(data);
	d->result = data.toVariant();
	d->type = "QScriptBreakpointData";
}

void QScriptDebuggerResponse::setResult(const QScriptBreakpointMap &breakpoints)
{
    Q_D(QScriptDebuggerResponse);
    //d->result = QVariant::fromValue(breakpoints);
	QVariantList result;
	for(QScriptBreakpointMap::const_iterator I = breakpoints.begin(); I != breakpoints.end(); ++I)
	{
		QVariantMap out = I.value().toVariant();
		out["id"] = I.key();
		result.append(out);
	}
	d->result = result;
	d->type = "QScriptBreakpointMap";
}

void QScriptDebuggerResponse::setResult(const QScriptScriptMap &scripts)
{
    Q_D(QScriptDebuggerResponse);
    //d->result = QVariant::fromValue(scripts);
	QVariantList result;
	for(QScriptScriptMap::const_iterator I = scripts.begin(); I != scripts.end(); ++I)
	{
		QVariantMap out = I.value().toVariant();
		out["id"] = I.key();
		result.append(out);
	}
	d->result = result;
	d->type = "QScriptScriptMap";
}

void QScriptDebuggerResponse::setResult(const QScriptScriptData &data)
{
    Q_D(QScriptDebuggerResponse);
    //d->result = QVariant::fromValue(data);
	QVariantMap result;
	result["contents"] = data.contents();
	result["fileName"] = data.fileName();
	result["baseLineNumber"] = data.baseLineNumber();
	result["timeStamp"] = data.timeStamp().toSecsSinceEpoch();
	d->result = result;
	d->type = "QScriptScriptData";
}

void QScriptDebuggerResponse::setResult(const QScriptDebuggerValue &value)
{
    Q_D(QScriptDebuggerResponse);
    //d->result = QVariant::fromValue(value);
	d->result = value.toVariant();
	d->type = "QScriptDebuggerValue";
}

void QScriptDebuggerResponse::setResult(const QScriptDebuggerValueList &values)
{
    Q_D(QScriptDebuggerResponse);
	//d->result = QVariant::fromValue(values);
    QVariantList result;
	for(QScriptDebuggerValueList::const_iterator I = values.begin(); I != values.end(); ++I)
		result.append(I->toVariant());
	d->result = result;
	d->type = "QScriptDebuggerValueList";
}

void QScriptDebuggerResponse::setResult(const QScriptDebuggerValuePropertyList &props)
{
    Q_D(QScriptDebuggerResponse);
    //d->result = QVariant::fromValue(props);
    QVariantList result;
	for(QScriptDebuggerValuePropertyList::const_iterator I = props.begin(); I != props.end(); ++I)
		result.append(I->toVariant());
	d->result = result;
	d->type = "QScriptDebuggerValuePropertyList";
}

//void QScriptDebuggerResponse::setResult(const QScriptContextInfo &info)
//{
//    Q_D(QScriptDebuggerResponse);
//    d->result = QVariant::fromValue(info);
//}

int QScriptDebuggerResponse::resultAsInt() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->result.toInt();
}

qint64 QScriptDebuggerResponse::resultAsLongLong() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->result.toLongLong();
}

QString QScriptDebuggerResponse::resultAsString() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->result.toString();
}

QScriptBreakpointData QScriptDebuggerResponse::resultAsBreakpointData() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptBreakpointData>(d->result);
	QScriptBreakpointData data;
	data.fromVariant(d->result.toMap());
	return data;
}

QScriptBreakpointMap QScriptDebuggerResponse::resultAsBreakpoints() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptBreakpointMap>(d->result);
	QScriptBreakpointMap breakpoints;
	foreach(const QVariant& var, d->result.toList())
	{
		QVariantMap in = var.toMap();
		QScriptBreakpointData data;
		data.fromVariant(in);
		breakpoints.insert(in["id"].toInt(), data);
	}
	return breakpoints;
}

QScriptScriptMap QScriptDebuggerResponse::resultAsScripts() const
{
    Q_D(const QScriptDebuggerResponse);
	//return qvariant_cast<QScriptScriptMap>(d->result);
	QScriptScriptMap scripts;
	foreach(const QVariant& var, d->result.toList())
	{
		QVariantMap in = var.toMap();
		QScriptScriptData data;
		data.fromVariant(in);
		scripts.insert(in["id"].toInt(), data);
	}
	return scripts;
}

QScriptScriptData QScriptDebuggerResponse::resultAsScriptData() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptScriptData>(d->result);
	QVariantMap result = d->result.toMap();
	return QScriptScriptData(result["contents"].toString(),
								result["fileName"].toString(),
								result["baseLineNumber"].toInt(), 
								QDateTime::fromSecsSinceEpoch(result["timeStamp"].toULongLong()));
}

QScriptDebuggerValue QScriptDebuggerResponse::resultAsScriptValue() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptDebuggerValue>(d->result);
	QScriptDebuggerValue value;
	value.fromVariant(d->result.toMap());
	return value;
}

QScriptDebuggerValueList QScriptDebuggerResponse::resultAsScriptValueList() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptDebuggerValueList>(d->result);
	QScriptDebuggerValueList list;
	foreach(const QVariant& var, d->result.toList())
	{
		QScriptDebuggerValue data;
		data.fromVariant(var.toMap());
		list.append(data);
	}
	return list;
}

QScriptDebuggerValuePropertyList QScriptDebuggerResponse::resultAsScriptValuePropertyList() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptDebuggerValuePropertyList>(d->result);
	QScriptDebuggerValuePropertyList list;
	foreach(const QVariant& var, d->result.toList())
	{
		QScriptDebuggerValueProperty data;
		data.fromVariant(var.toMap());
		list.append(data);
	}
	return list;
}

QScriptDebuggerContextInfo QScriptDebuggerResponse::resultAsContextInfo() const
{
    Q_D(const QScriptDebuggerResponse);
    //return qvariant_cast<QScriptContextInfo>();
	QScriptDebuggerContextInfo info;
	info.fromVariant(d->result.toMap());
	return info;
}

bool QScriptDebuggerResponse::async() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->async;
}

void QScriptDebuggerResponse::setAsync(bool async)
{
    Q_D(QScriptDebuggerResponse);
    d->async = async;
}

/*!
  Returns true if this QScriptDebuggerResponse is equal to the \a other
  response, otherwise returns false.
*/
bool QScriptDebuggerResponse::operator==(const QScriptDebuggerResponse &other) const
{
    Q_D(const QScriptDebuggerResponse);
    const QScriptDebuggerResponsePrivate *od = other.d_func();
    if (d == od)
        return true;
    if (!d || !od)
        return false;
    return ((d->error == od->error)
            && (d->result == od->result)
            && (d->async == od->async));
}

/*!
  Returns true if this QScriptDebuggerResponse is not equal to the \a
  other response, otherwise returns false.
*/
bool QScriptDebuggerResponse::operator!=(const QScriptDebuggerResponse &other) const
{
    return !(*this == other);
}

//> NeoScriptTools
void QScriptDebuggerResponse::fromVariant(const QVariant& var)
{
	QVariantMap in = var.toMap();
    QScriptDebuggerResponsePrivate *d = d_ptr.data();

	Error error = NoError;
	QVariant err = in["error"];
	if (err.isValid()) {
		if (err.type() == QVariant::String) {
			if (err == "InvalidContextIndex") error = InvalidContextIndex;
			else if (err == "InvalidArgumentIndex") error = InvalidArgumentIndex;
			else if (err == "InvalidScriptID") error = InvalidScriptID;
			else if (err == "InvalidBreakpointID") error = InvalidBreakpointID;
			else if (err == "DetachedError") error = DetachedError;
			else if (err == "UserError") error = UserError;
			else error = MaxUserError;
		} else
			err = err.toInt();
	}
    d->error = error;

    d->result = in["result"];
	d->type = in["type"].toString();
	d->async = in["async"].toBool();
}

QVariant QScriptDebuggerResponse::toVariant() const
{
	QVariantMap out;

    const QScriptDebuggerResponsePrivate *d = d_ptr.data();
	switch(d->error){
	case NoError: break;
	case InvalidContextIndex: out["error"] = "InvalidContextIndex"; break;
	case InvalidArgumentIndex: out["error"] = "InvalidArgumentIndex"; break;
	case InvalidScriptID: out["error"] = "InvalidScriptID"; break;
	case InvalidBreakpointID: out["error"] = "InvalidBreakpointID"; break;
	case DetachedError: out["error"] = "DetachedError"; break;
	case UserError: out["error"] = "UserError"; break;
	default: 
		Q_ASSERT(d->error > UserError);
		out["error"] = (int)d->error; break;
	}

    out["result"] = d->result;
	out["type"] = d->type;
    out["async"] = d->async;

    return out;
}
//< NeoScriptTools

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptDebuggerResponse &response)
  \relates QScriptDebuggerResponse

  Writes the given \a response to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptDebuggerResponse &response)
{
    const QScriptDebuggerResponsePrivate *d = response.d_ptr.data();
    out << (quint32)d->error;
    out << d->result;
	out << d->type;
    out << d->async;
    return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptDebuggerResponse &response)
  \relates QScriptDebuggerResponse

  Reads a QScriptDebuggerResponse from the specified \a stream into the
  given \a response.
*/
QDataStream &operator>>(QDataStream &in, QScriptDebuggerResponse &response)
{
    QScriptDebuggerResponsePrivate *d = response.d_ptr.data();

    quint32 error;
    in >> error;
    d->error = QScriptDebuggerResponse::Error(error);
    in >> d->result;
	in >> d->type;
    in >> d->async;

    return in;
}

QT_END_NAMESPACE
