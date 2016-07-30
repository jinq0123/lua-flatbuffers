#include "xxx_encoder.h"

using flatbuffers::uoffset_t;

// Todo: check required fields.
// Todo: Skip default value.

uoffset_t XXXEncoder::EncodeObject(const Object& obj, const LuaRef& luaTable)
{
	return obj.is_struct() ?
		EncodeStruct(obj, luaTable) :
		EncodeTable(obj, luaTable);
}

uoffset_t XXXEncoder::EncodeStruct(const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	(void)Builder().StartStruct(obj.minalign());
	uint8_t* pBuf = Builder().ReserveElements(obj.bytesize(), 1);
	assert(pBuf);
	if (!EncodeStructToBuf(obj, luaTable, pBuf))
		return 0;
	return Builder().EndStruct();
}

bool XXXEncoder::EncodeStructToBuf(const Object& obj,
	const LuaRef& luaTable, uint8_t* pBuf)
{
	assert(pBuf);
	// Struct should traverse all fields of object.
	// Lua table traverse is better, to check fields count.
	for (const Field* pField : *obj.fields())
	{
		assert(pField);
		assert(!pField->deprecated());  // Struct has no deprecated field.
		if (!EncodeStructFieldToBuf(*pField, luaTable, pBuf))
			return false;
	}  // for
	return true;
}  // EncodeStructToBuf()

bool XXXEncoder::CheckStructFields(const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	const auto& vFields = *obj.fields();
	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField)
			ERR_RET_FALSE("illegal field " + PopFullFieldName(sKey));
		assert(!pField->deprecated());
	}
	return true;
}  // CheckStructFields()

bool XXXEncoder::EncodeStructFieldToBuf(const Field& field,
	const LuaRef& luaTable, uint8_t* pBuf)
{
	assert(pBuf);
	const char* pFieldName = field.name()->c_str();
	const LuaRef luaValue = luaTable.get(pFieldName);
	if (!luaValue)
		ERR_RET_FALSE("missing struct field " + PopFullFieldName(pFieldName));

	const reflection::Type& type = *field.type();
	// Todo: check type of value...

	uint16_t offset = field.offset();
	uint8_t* pDest = pBuf + offset;
	reflection::BaseType eBaseType = type.base_type();
	if (eBaseType <= reflection::Double)
	{
		EncodeStructElementToBuf(eBaseType, luaValue, pBuf + offset);  // XXX throw?
		return true;
	}

	assert(eBaseType == reflection::Obj);
	const Object* pFieldObj = Objects()[type.index()];
	assert(pFieldObj);
	assert(pFieldObj->is_struct());
	PushName(pFieldName);
	if (!EncodeStructToBuf(*pFieldObj, luaValue, pDest))
		return false;
	SafePopName();
	return true;
}  // EncodeStructFieldToBuf()

template <typename T>
static void CopyToBuf(const LuaIntf::LuaRef& luaValue, uint8_t* pDest)
{
	T val = luaValue.toValue<T>();  // Todo: throw?
	*reinterpret_cast<T*>(pDest) = val;
}

// Encode struct scalar element to buffer.
void XXXEncoder::EncodeStructElementToBuf(reflection::BaseType eType,
	const LuaRef& luaValue, uint8_t* pDest)
{
	assert(pDest);
	switch (eType)
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		CopyToBuf<uint8_t>(luaValue, pDest);
		break;
	case reflection::Byte:
		CopyToBuf<int8_t>(luaValue, pDest);
		break;
	case reflection::Short:
		CopyToBuf<int16_t>(luaValue, pDest);
		break;
	case reflection::UShort:
		CopyToBuf<uint16_t>(luaValue, pDest);
		break;
	case reflection::Int:
		CopyToBuf<int32_t>(luaValue, pDest);
		break;
	case reflection::UInt:
		CopyToBuf<uint32_t>(luaValue, pDest);
		break;
	case reflection::Long:
		CopyToBuf<int64_t>(luaValue, pDest);
		break;
	case reflection::ULong:
		CopyToBuf<uint64_t>(luaValue, pDest);
		break;
	case reflection::Float:
		CopyToBuf<float>(luaValue, pDest);
		break;
	case reflection::Double:
		CopyToBuf<double>(luaValue, pDest);
		break;
	}
	assert(!"Illegal type.");
}

uoffset_t XXXEncoder::EncodeTable(const Object& obj, const LuaRef& luaTable)
{
	assert(!obj.is_struct());
	// Cache to map before StartTable().
	Field2Lua mapScalar;
	Field2Offset mapOffset;
	if (!CacheFields(obj, luaTable, mapScalar, mapOffset))
		return 0;

	uoffset_t start = Builder().StartTable();
	AddElements(mapScalar);
	AddOffsets(mapOffset);
	return Builder().EndTable(start, obj.fields()->size());
}

uoffset_t XXXEncoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	assert(type.base_type() == reflection::Vector);
	// todo: check luaArray is array
	return 0;
}

// Cache fields to 2 maps.
bool XXXEncoder::CacheFields(const Object& obj, const LuaRef& luaTable,
	Field2Lua& rMapLuaRef, Field2Offset& rMapOffset)
{
	const auto& vFields = *obj.fields();
	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!CheckObjectField(pField, sKey))
			return false;
		assert(pField);

		LuaRef value = e.value<LuaRef>();
		CacheField(pField, value, rMapLuaRef, rMapOffset);
	}
	return true;
}

// Cache field to 2 maps.
void XXXEncoder::CacheField(const Field* pField, const LuaRef& luaValue,
	Field2Lua& rMapLuaRef, Field2Offset& rMapOffset)
{
	assert(pField);
	const reflection::Type& type = *pField->type();
	// Todo: check type of value...
	switch (type.base_type())
	{
	case reflection::String:
		rMapOffset[pField] = Builder().CreateString(
			luaValue.toValue<const char*>()).o;
		break;
	case reflection::Vector:
		rMapOffset[pField] = EncodeVector(type, luaValue);
		break;
	case reflection::Obj:
	{
		const Object* pObj = Objects()[type.index()];
		assert(pObj);
		if (pObj->is_struct()) rMapLuaRef[pField] = luaValue;
		else rMapOffset[pField] = EncodeObject(*pObj, luaValue);
		break;
	}
	case reflection::Union:
		// XXX get union underlying type...
		break;
	default:
		rMapLuaRef[pField] = luaValue;
		break;
	}  // switch
}

void XXXEncoder::AddElements(const Field2Lua& mapScalar)
{
	for (const auto& e : mapScalar)
	{
		const Field* pField = e.first;
		assert(pField);
		AddElement(*pField, e.second);
	}
}

void XXXEncoder::AddElement(const Field& field, const LuaRef& elementValue)
{
	const reflection::Type& type = *field.type();
	int64_t defInt = field.default_integer();
	double defReal = field.default_real();
	uint16_t offset = field.offset();

	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		AddElement<uint8_t>(offset, elementValue, defInt);
		break;
	case reflection::Byte:
		AddElement<int8_t>(offset, elementValue, defInt);
		break;
	case reflection::Short:
		AddElement<int16_t>(offset, elementValue, defInt);
		break;
	case reflection::UShort:
		AddElement<uint16_t>(offset, elementValue, defInt);
		break;
	case reflection::Int:
		AddElement<int32_t>(offset, elementValue, defInt);
		break;
	case reflection::UInt:
		AddElement<uint32_t>(offset, elementValue, defInt);
		break;
	case reflection::Long:
		AddElement<int64_t>(offset, elementValue, defInt);
		break;
	case reflection::ULong:
		AddElement<uint64_t>(offset, elementValue, defInt);
		break;
	case reflection::Float:
		AddElement<float>(offset, elementValue, defReal);
		break;
	case reflection::Double:
		AddElement<double>(offset, elementValue, defReal);
		break;
	case reflection::Obj:
		// XXX struct...
		break;
	default:
		assert(!"Illegal type.");
		break;
	}
}

void XXXEncoder::AddOffsets(const Field2Offset& mapOffset)
{
	for (const auto& e : mapOffset)
	{
		const Field* pField = e.first;
		assert(pField);
		uoffset_t offset = e.second;
		Builder().AddOffset(pField->offset(), flatbuffers::Offset<void>(offset));
	}
}

template <typename ElementType, typename DefaultValueType>
inline void XXXEncoder::AddElement(uint16_t offset,
	const LuaRef& elementValue, DefaultValueType defaultValue)
{
	Builder().AddElement(offset,
		elementValue.toValue<ElementType>(),
		static_cast<ElementType>(defaultValue));
}

// Set error and return false if field is illegal.
// sFieldName is only used for error message.
bool XXXEncoder::CheckObjectField(const Field* pField, const string& sFieldName)
{
	if (!pField)
		ERR_RET_FALSE("illegal field " + PopFullFieldName(sFieldName));
	if (pField->deprecated())
		ERR_RET_FALSE("deprecated field " + PopFullFieldName(sFieldName));
	return true;
}

