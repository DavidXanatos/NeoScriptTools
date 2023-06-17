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

#include "V4DebugHandler.h"

#include <private/qv4script_p.h>
#include <private/qv4string_p.h>
#include <private/qv4objectiterator_p.h>
#include <private/qv4runtime_p.h>
#include <private/qv4identifiertable_p.h>


void SV4Value::fromVariant(const QVariantMap& in)
{
    QString strType = in["type"].toString();
    if (strType == "UndefinedValue")
        type = "undefined";
    else if (strType == "NullValue")
        type = "null";
    else if (strType == "BooleanValue") {
        type = "boolean";
        data = in["value"];
    }
    else if (strType == "NumberValue") {
        type = "number";
        data = in["value"];
    }
    else if (strType == "StringValue") {
        type = "string";
        data = in["value"];
    }
    else if (strType == "ObjectValue") {
        type = "object";
        UV4Handle handle = { in["value"].toULongLong() };
        ref = handle.ref;
    }
}

QVariantMap SV4Value::toVariant() const
{
    QVariantMap value;
    if (type == "undefined")
        value["type"] = "UndefinedValue";
    else if (type == "null")
        value["type"] = "NullValue";
    else if (type == "function")
        value["type"] = "NoValue";
    else if (type == "boolean") {
        value["type"] = "BooleanValue";
        value["value"] = data;
    }
    else if (type == "number") {
        value["type"] = "NumberValue";
        value["value"] = data;
    }
    else if (type == "string") {
        value["type"] = "StringValue";
        value["value"] = data;
    }
    else if (type == "object") {
        value["type"] = "ObjectValue";
        UV4Handle handle = { 0 };
        handle.type = UV4Handle::eObject;
        handle.ref = ref;
        value["value"] = handle.value;
    }
    else
        value["type"] = "NoValue";
    return value;
}

void SV4Property::fromVariant(const QVariantMap& in)
{
    name = in["name"].toString();
    SV4Value::fromVariant(in["value"].toMap());
}

QVariantMap SV4Property::toVariant() const
{
    QVariantMap out;
    out["name"] = name;
    out["value"] = SV4Value::toVariant();
    if (type != "function")
        out["valueAsString"] = data.toString();
    out["flags"] = 0; // QScriptValue::PropertyFlag : ReadOnly = 0x00000001, Undeletable = 0x00000002
    return out;
}

////////////////////////////////////////////////////////////////////////////////////
// CV4DebugHandler
//

CV4DebugHandler::CV4DebugHandler(QV4::ExecutionEngine* engine, QObject* parent)
	: QObject(parent)
{
    m_engine = engine;
    m_refArray.set(engine, engine->newArrayObject());
}

const QV4::Object* CV4DebugHandler::getValue(const QV4::ScopedValue& value, SV4Value* result)
{
    QV4::Scope scope(m_engine);
    QV4::ScopedValue type(scope, QV4::Runtime::TypeofValue::call(m_engine, value));
    result->type = type->toQStringNoThrow();

    switch (value->type()) {
    case QV4::Value::Managed_Type:
        if (const QV4::ArrayObject* arr = value->as<QV4::ArrayObject>()) {
            result->data = qint64(arr->getLength());
            return arr;
        }
        else if (const QV4::Object* obj = value->as<QV4::Object>()) {
            QV4::ObjectIterator it(scope, obj, QV4::ObjectIterator::EnumerableOnly);
            QV4::PropertyAttributes attrs;
            QV4::ScopedPropertyKey name(scope);
            int count = 0;
            for (;; count++) {
                name = it.next(nullptr, &attrs);
                if (!name->isValid())
                    break;
            }
            result->data = count;
            return obj;
        }
        else if (const QV4::String* str = value->as<QV4::String>())
            result->data = str->toQString();
        return nullptr;
    case QV4::Value::Boolean_Type:
        result->data = value->booleanValue();
        return nullptr;
    case QV4::Value::Integer_Type:
        result->data = value->integerValue();
        return nullptr;
    case QV4::Value::Double_Type:
        result->data = value->doubleValue();
        return nullptr;
    default:
    case QV4::Value::Empty_Type:
    case QV4::Value::Undefined_Type:
    case QV4::Value::Null_Type:
        return nullptr;
    }
}
QVector<SV4Property> CV4DebugHandler::getProperties(const QV4::Object* object)
{
    QVector<SV4Property> properties;

    QV4::Scope scope(m_engine);
    QV4::ObjectIterator it(scope, object, QV4::ObjectIterator::EnumerableOnly);
    QV4::ScopedValue name(scope);
    QV4::ScopedValue value(scope);
    for (;;) {
        QV4::Value v;
        name = it.nextPropertyNameAsString(&v);
        if (name->isNull())
            break;
        value = v;

        SV4Property result;
        QString key = name->toQStringNoThrow();
        if (!key.isNull())
            result.name = key;

        getValue(value, &result);
        if (value->isManaged() && !value->isString())
            result.ref = addRef(value);

        properties.append(result);
    }

    return properties;
}

SV4Object CV4DebugHandler::getObject(const QV4::ScopedValue& value, uint ref)
{
    SV4Object result;

    result.ref = ref;
    const QV4::Object* object = getValue(value, &result);
    if (object) {
        result.handle.type = UV4Handle::eObject;
        result.properties = getProperties(object);
    } else
        result.handle.type = UV4Handle::eValue;

    return result;
}

SV4Object CV4DebugHandler::lookupRef(uint ref)
{
    QV4::Scope scope(m_engine);
    QV4::ScopedValue value(scope, getValue(ref));

    return getObject(value, ref);
}

bool CV4DebugHandler::isValidRef(uint ref) const
{
    QV4::Scope scope(m_engine);
    QV4::ScopedObject refArray(scope, m_refArray.value());

    return ref < refArray->getLength();
}

uint CV4DebugHandler::addRef(QV4::Value value)
{
    QV4::Scope scope(m_engine);
    QV4::ScopedObject refArray(scope, m_refArray.value());
    
    // find and return already existing object
    for (uint i = 0; i < refArray->getLength(); ++i) {
        if (refArray->get(i) == value.rawValue())
            return i;
    }
 
    quint8 hadException = m_engine->hasException;
    m_engine->hasException = false; // ensure put works

    // track new object
    uint ref = refArray->getLength();
    refArray->put(ref, value);
    Q_ASSERT(refArray->getLength() - 1 == ref);

    m_engine->hasException = hadException;

    return ref;
}

QV4::ReturnedValue CV4DebugHandler::getValue(uint ref)
{
    QV4::Scope scope(m_engine);
    QV4::ScopedObject refArray(scope, m_refArray.value());

    Q_ASSERT(ref < refArray->getLength());
    return refArray->get(ref, nullptr);
}
