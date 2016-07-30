#include "struct_encoder.h"

using flatbuffers::uoffset_t;

// Todo: check required fields.
// Todo: Skip default value.

uoffset_t StructEncoder::EncodeStruct(const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	(void)Builder().StartStruct(obj.minalign());
	uint8_t* pBuf = Builder().ReserveElements(obj.bytesize(), 1);
	assert(pBuf);
	if (!EncodeStructToBuf(obj, luaTable, pBuf))
		return 0;
	return Builder().EndStruct();
}

bool StructEncoder::EncodeStructToBuf(const Object& obj,
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

bool StructEncoder::CheckStructFields(const Object& obj, const LuaRef& luaTable)
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

bool StructEncoder::EncodeStructFieldToBuf(const Field& field,
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
void StructEncoder::EncodeStructElementToBuf(reflection::BaseType eType,
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

