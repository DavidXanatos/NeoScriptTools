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

#ifndef CV4DEBUGHANDLER_H
#define CV4DEBUGHANDLER_H

#include <QObject>

#include <private/qv4engine_p.h>
#include <private/qv4persistent_p.h>


union UV4Handle
{
	quint64 value;				// 64
	enum {
		eValue = 0,
		eScope,
		eObject,
		eThis
	};
	struct {
		quint32					// 32
			type : 8,
			unused : 24;
		union {
			struct {
				short frame;	// 16
				short scope;	// 16
			};					// 32
			uint ref;			// 32
		};						// 32
	};							// 64
};

struct SV4Value
{
	SV4Value() : ref(-1) {}

	void fromVariant(const QVariantMap& in);
    QVariantMap toVariant() const;

	QString type;
	QVariant data;
	int ref;
};

struct SV4Property : SV4Value
{
	void fromVariant(const QVariantMap& in);
	QVariantMap toVariant() const;

	SV4Property() {}
	SV4Property(const SV4Value& v, const QString n) 
		: SV4Value(v), name(n) {}

	QString name;
};

struct SV4Object: SV4Value
{
	SV4Object() : handle{ 0 } {}

	UV4Handle			handle;
	QVector<SV4Property>properties;
};

struct SV4ValueIterator
{
	SV4ValueIterator() : index(0) {}

	SV4Object	snapshot;
	int					index;
};

////////////////////////////////////////////////////////////////////////////////////
// CV4DebugHandler
//

class CV4DebugHandler : public QObject
{
    Q_OBJECT

public:
    CV4DebugHandler(QV4::ExecutionEngine* engine, QObject* parent = nullptr);

    QV4::ExecutionEngine* engine() const { return m_engine; }

	uint addRef(QV4::Value value);
	QV4::ReturnedValue getValue(uint ref);

    bool isValidRef(uint ref) const;
	SV4Object lookupRef(uint ref);

protected:
	friend class CV4GetPropsJob;
	const QV4::Object* getValue(const QV4::ScopedValue& value, SV4Value* result);
	QVector<SV4Property> getProperties(const QV4::Object* object);
	SV4Object getObject(const QV4::ScopedValue& value, uint ref);

private:
    QV4::ExecutionEngine* m_engine;
    QV4::PersistentValue m_refArray;
};

#endif