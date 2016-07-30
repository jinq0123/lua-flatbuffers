#include "table_encoder.h"

#include "struct_encoder.h"  // StructEncoder

// Todo: check required fields.
// Todo: Skip default value.

flatbuffers::uoffset_t TableEncoder::EncodeTable(
	const Object& obj, const LuaRef& luaTable)
{
	assert(!obj.is_struct());
	assert(luaTable.isTable());

	// Cache to map before StartTable().
	m_mapStructs.clear();
	m_mapScalars.clear();
	m_mapOffsets.clear();
	CacheFields(obj, luaTable);
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

flatbuffers::uoffset_t TableEncoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	assert(type.base_type() == reflection::Vector);
	// todo: check luaArray is array
	return 0;
}

// Cache fields to 3 maps.
void TableEncoder::CacheFields(const Object& obj, const LuaRef& luaTable)
{
	const auto& vFields = *obj.fields();
	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		CheckObjectField(pField, sKey);
		if (Bad()) return;
		assert(pField);

		LuaRef value = e.value<LuaRef>();
		CacheField(pField, value);
		if (Bad()) return;
	}
}

// Cache field to 3 maps.
void TableEncoder::CacheField(const Field* pField, const LuaRef& luaValue)
{
	assert(pField);
	using namespace reflection;
	const Type& type = *pField->type();
	// Todo: check type of value...
	switch (type.base_type())
	{
	case String:
		m_mapOffsets[pField] = Builder().CreateString(
			luaValue.toValue<const char*>()).o;
		break;
	case Vector:
		m_mapOffsets[pField] = EncodeVector(type, luaValue);
		break;
	case Obj:
		CacheObjField(pField, luaValue);
		break;
	case Union:
		// XXX get union underlying type...
		break;
	default:
		m_mapScalars[pField] = luaValue;
		break;
	}  // switch
}

void TableEncoder::CacheObjField(const Field* pField, const LuaRef& luaValue)
{
	assert(pField);

	if (!luaValue.isTable())
	{
		ERR_RET("object " + PopFullFieldName(pField->name()->c_str())
			+ " is not a table but " + luaValue.typeName());
	}

	const reflection::Type& type = *pField->type();
	assert(reflection::Obj == type.base_type());
	const Object* pObj = Objects()[type.index()];
	assert(pObj);
	if (pObj->is_struct())
		m_mapStructs[pField] = luaValue;
	else
		m_mapOffsets[pField] = TableEncoder(m_rCtx)
			.EncodeTable(*pObj, luaValue);
}

void TableEncoder::EncodeCachedStructs()
{
	for (const auto& e : m_mapStructs)
	{
		const Field* pField = e.first;
		const LuaRef luaValue = e.second;
		assert(pField);
		EncodeStruct(*pField, luaValue);
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
		EncodeScalar(*pField, e.second);
	}
}

void TableEncoder::EncodeScalar(const Field& field, const LuaRef& luaValue)
{
	using namespace reflection;
	const Type& type = *field.type();
	int64_t defInt = field.default_integer();
	double defReal = field.default_real();
	uint16_t offset = field.offset();

	assert(type.base_type() <= Double);  // Only scalar.
	switch (type.base_type())
	{
	case UType:
	case Bool:
	case UByte:
		AddElement<uint8_t>(offset, luaValue, defInt);
		break;
	case Byte:
		AddElement<int8_t>(offset, luaValue, defInt);
		break;
	case Short:
		AddElement<int16_t>(offset, luaValue, defInt);
		break;
	case UShort:
		AddElement<uint16_t>(offset, luaValue, defInt);
		break;
	case Int:
		AddElement<int32_t>(offset, luaValue, defInt);
		break;
	case UInt:
		AddElement<uint32_t>(offset, luaValue, defInt);
		break;
	case Long:
		AddElement<int64_t>(offset, luaValue, defInt);
		break;
	case ULong:
		AddElement<uint64_t>(offset, luaValue, defInt);
		break;
	case Float:
		AddElement<float>(offset, luaValue, defReal);
		break;
	case Double:
		AddElement<double>(offset, luaValue, defReal);
		break;
	default:
		assert(!"Illegal type.");
		break;
	}
}

void TableEncoder::EncodeCachedOffsets()
{
	for (const auto& e : m_mapOffsets)
	{
		const Field* pField = e.first;
		assert(pField);
		uoffset_t offset = e.second;
		Builder().AddOffset(pField->offset(),
			flatbuffers::Offset<void>(offset));
	}
}

template <typename ElementType, typename DefaultValueType>
inline void TableEncoder::AddElement(uint16_t offset,
	const LuaRef& elementValue, DefaultValueType defaultValue)
{
	Builder().AddElement(offset,
		elementValue.toValue<ElementType>(),
		static_cast<ElementType>(defaultValue));
}

// Set error if field is illegal.
// sFieldName is only used for error message.
void TableEncoder::CheckObjectField(const Field* pField, const string& sFieldName)
{
	if (!pField)
		ERR_RET("illegal field " + PopFullFieldName(sFieldName));
	if (pField->deprecated())
		ERR_RET("deprecated field " + PopFullFieldName(sFieldName));
}
