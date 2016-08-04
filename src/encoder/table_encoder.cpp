#include "table_encoder.h"

#include "struct_encoder.h"  // for StructEncoder
#include "union_encoder.h"  // for UnionEncoder
#include "vector_encoder.h"  // for VectorEncoder

flatbuffers::uoffset_t TableEncoder::EncodeTable(
	const Object& obj, const LuaRef& luaTable)
{
	assert(!obj.is_struct());
	assert(luaTable.isTable());
	m_pLuaTable = &luaTable;
	CheckRequiredFields(obj);
	if (Bad()) return 0;

	// Cache to map before StartTable().
	m_mapStructs.clear();
	m_mapScalars.clear();
	m_mapOffsets.clear();
	CacheFields(obj);
	if (Bad()) return 0;

	uoffset_t start = Builder().StartTable();
	EncodeCachedStructs();
	if (Bad()) return 0;
	EncodeCachedScalars();
	if (Bad()) return 0;
	EncodeCachedOffsets();
	if (Bad()) return 0;
	return Builder().EndTable(start, obj.fields()->size());
}

// Cache fields to 3 maps.
void TableEncoder::CacheFields(const Object& obj)
{
	const auto& vFields = *obj.fields();
	for (const auto& e : *m_pLuaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField)
			ERR_RET("illegal field " + PopFullFieldName(sKey));
		if (pField->deprecated())
			ERR_RET("deprecated field " + PopFullFieldName(sKey));

		LuaRef value = e.value<LuaRef>();
		CacheField(*pField, value);
		if (Bad()) return;
	}
}

// Cache field to 3 maps.
void TableEncoder::CacheField(const Field& field, const LuaRef& luaValue)
{
	using namespace reflection;
	const Type& type = *field.type();
	switch (type.base_type())
	{
	case String: return CacheStringField(field, luaValue);
	case Vector: return CacheVectorField(field, luaValue);
	case Obj: return CacheObjField(field, luaValue);
	case Union: return CacheUnionField(field, luaValue);
	}  // switch

	m_mapScalars[&field] = luaValue;
}

void TableEncoder::CacheStringField(const Field& field, const LuaRef& luaValue)
{
	LuaIntf::LuaTypeID luaTypeId = luaValue.type();
	if (luaTypeId == LuaIntf::LuaTypeID::STRING ||
		luaTypeId == LuaIntf::LuaTypeID::NUMBER)
	{
		m_mapOffsets[&field] = Builder().CreateString(
			luaValue.toValue<const char*>()).o;
		return;
	}
	SetError("string field " + PopFullFieldName(field)
		+ " is " + luaValue.typeName());
}

void TableEncoder::CacheVectorField(const Field& field, const LuaRef& luaValue)
{
	const reflection::Type& type = *field.type();
	assert(reflection::Vector == type.base_type());
	PushName(field);  // Todo: Need array index.
	m_mapOffsets[&field] = VectorEncoder(m_rCtx).EncodeVector(type, luaValue);
	SafePopName();
}

void TableEncoder::CacheObjField(const Field& field, const LuaRef& luaValue)
{
	if (!luaValue.isTable())
	{
		ERR_RET("object " + PopFullFieldName(field.name()->c_str())
			+ " is not a table but " + luaValue.typeName());
	}

	const reflection::Type& type = *field.type();
	assert(reflection::Obj == type.base_type());
	const Object* pObj = Objects()[type.index()];
	assert(pObj);
	if (pObj->is_struct())
	{
		m_mapStructs[&field] = luaValue;
		return;
	}

	PushName(field);
	m_mapOffsets[&field] = TableEncoder(m_rCtx).EncodeTable(*pObj, luaValue);
	SafePopName();
}

void TableEncoder::CacheUnionField(const Field& field, const LuaRef& luaValue)
{
	const reflection::Type& type = *field.type();
	assert(reflection::Union == type.base_type());
	const reflection::Enum* pEnum = (*m_rCtx.schema.enums())[type.index()];
	assert(pEnum);
	string sFieldName(field.name()->c_str());
	string sTypeField = sFieldName + flatbuffers::UnionTypeFieldSuffix();
	LuaRef luaType = m_pLuaTable->get(sTypeField);
	PushName(field);
	m_mapOffsets[&field] =
		UnionEncoder(m_rCtx).EncodeUnion(*pEnum, luaType, luaValue);
	SafePopName();
}

void TableEncoder::EncodeCachedStructs()
{
	for (const auto& e : m_mapStructs)
	{
		const Field* pField = e.first;
		const LuaRef luaValue = e.second;
		assert(pField);
		PushName(*pField);
		EncodeStruct(*pField, luaValue);
		SafePopName();
		if (Bad()) return;
	}
}

void TableEncoder::EncodeStruct(const Field& field, const LuaRef& luaValue)
{
	const reflection::Type& type = *field.type();
	const Object* pObj = Objects()[type.index()];
	assert(pObj);
	assert(pObj->is_struct());
	uoffset_t offset = StructEncoder(m_rCtx).EncodeStruct(*pObj, luaValue);
	Builder().AddStructOffset(field.offset(), offset);
}

void TableEncoder::EncodeCachedScalars()
{
	for (const auto& e : m_mapScalars)
	{
		const Field* pField = e.first;
		assert(pField);
		const LuaRef& luaVal = e.second;

		PushName(*pField);
		EncodeScalar(*pField, e.second);
		if (Bad()) return;
		SafePopName();
	}
}

static bool IsEnumType(const reflection::Type& type)
{
	// enum must be int
	return type.index() >= 0 && type.base_type() < reflection::Float;
}

void TableEncoder::EncodeScalar(const Field& field, const LuaRef& luaValue)
{
	using namespace reflection;
	const Type& type = *field.type();
	if (IsEnumType(type) && luaValue.type() == LuaIntf::LuaTypeID::STRING)
		return EncodeStringEnum(field, luaValue);  // Allow string eunm.

	int64_t defInt = field.default_integer();
	double defReal = field.default_real();
	uint16_t offset = field.offset();

	BaseType eType = type.base_type();
	assert(eType <= Double);  // Only scalar.
	switch (eType)
	{
	case UType:
	case Bool:  // true -> 1
	case UByte: return AddElement<uint8_t>(offset, luaValue, defInt);
	case Byte: return AddElement<int8_t>(offset, luaValue, defInt);
	case Short: return AddElement<int16_t>(offset, luaValue, defInt);
	case UShort: return AddElement<uint16_t>(offset, luaValue, defInt);
	case Int: return AddElement<int32_t>(offset, luaValue, defInt);
	case UInt: return AddElement<uint32_t>(offset, luaValue, defInt);
	case Long: return AddElement<int64_t>(offset, luaValue, defInt);
	case ULong: return AddElement<uint64_t>(offset, luaValue, defInt);
	case Float: return AddElement<float>(offset, luaValue, defReal);
	case Double: return AddElement<double>(offset, luaValue, defReal);
	}
	assert(!"Illegal type.");
}

void TableEncoder::EncodeStringEnum(const Field& field, const LuaRef& luaValue)
{
	using namespace reflection;
	const Type& type = *field.type();
	int64_t lEnumVal = GetEnumFromLuaStr(type, luaValue);
	if (Bad()) return;

	int64_t defInt = field.default_integer();
	uint16_t offset = field.offset();
	switch (type.base_type())
	{
	case UType:
	case Bool:  // true -> 1
	case UByte: return AddIntElement<uint8_t>(offset, lEnumVal, defInt);
	case Byte: return AddIntElement<int8_t>(offset, lEnumVal, defInt);
	case Short: return AddIntElement<int16_t>(offset, lEnumVal, defInt);
	case UShort: return AddIntElement<uint16_t>(offset, lEnumVal, defInt);
	case Int: return AddIntElement<int32_t>(offset, lEnumVal, defInt);
	case UInt: return AddIntElement<uint32_t>(offset, lEnumVal, defInt);
	case Long: return AddIntElement<int64_t>(offset, lEnumVal, defInt);
	case ULong: return AddIntElement<uint64_t>(offset, lEnumVal, defInt);
	}
	assert(!"Illegal enum type.");
}

void TableEncoder::EncodeCachedOffsets()
{
	assert(!Bad());
	for (const auto& e : m_mapOffsets)
	{
		const Field* pField = e.first;
		assert(pField);
		uoffset_t offset = e.second;
		Builder().AddOffset(pField->offset(),
			flatbuffers::Offset<void>(offset));
	}
	assert(!Bad());
}

template <typename ElementType, typename DefaultValueType>
inline void TableEncoder::AddElement(uint16_t offset,
	const LuaRef& elementValue, DefaultValueType defaultValue)
{
	static_assert(std::is_scalar<ElementType>::value,
		"AddElement() is only for scalar types.");
	ElementType val = LuaToNumber<ElementType>(elementValue);
	if (Bad()) return;
	Builder().AddElement(offset, val,
		static_cast<ElementType>(defaultValue));
}

template <typename ElementType>
inline void TableEncoder::AddIntElement(uint16_t offset,
	int64_t lValue, int64_t lDefault)
{
	static_assert(std::is_scalar<ElementType>::value,
		"AddIntElement() is only for int types.");
	Builder().AddElement(offset,
		static_cast<ElementType>(lValue),
		static_cast<ElementType>(lDefault));
}

int64_t TableEncoder::GetEnumFromLuaStr(
	const reflection::Type& type, const LuaRef& luaValue)
{
	assert(IsEnumType(type));
	assert(luaValue.type() == LuaIntf::LuaTypeID::STRING);
	string sEnumVal = luaValue.toValue<string>();
	const reflection::Enum* pEnum = (*m_rCtx.schema.enums())[type.index()];
	assert(pEnum);
	for (const reflection::EnumVal* pEnumVal : *pEnum->values())
	{
		assert(pEnumVal);
		if (pEnumVal->name()->c_str() == sEnumVal)
			return pEnumVal->value();
	}
	SetError(string("illegal enum ") + pEnum->name()->str() + " field "
		+ PopFullName() + "(" + sEnumVal + ")");
	return 0;
}

void TableEncoder::CheckRequiredFields(const Object& obj)
{
	assert(m_pLuaTable);
	for (const Field* pField : *obj.fields())
	{
		assert(pField);
		if (pField->required() &&
			!m_pLuaTable->has(pField->name()->c_str()))
			ERR_RET("missing required field " + PopFullFieldName(*pField));
	}
}

