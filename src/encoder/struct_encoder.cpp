#include "struct_encoder.h"

flatbuffers::uoffset_t StructEncoder::EncodeStruct(
	const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	CheckLuaTable(obj, luaTable);
	if (Bad()) return 0;

	(void)Builder().StartStruct(obj.minalign());
	uint8_t* pBuf = Builder().ReserveElements(obj.bytesize(), 1);
	assert(pBuf);
	EncodeStructToBuf(obj, luaTable, pBuf);
	if (Bad()) return 0;
	return Builder().EndStruct();
}

void StructEncoder::EncodeStructToBuf(const Object& obj,
	const LuaRef& luaTable, uint8_t* pBuf)
{
	assert(pBuf);
	// Struct should traverse all fields of object.
	for (const Field* pField : *obj.fields())
	{
		assert(pField);
		assert(!pField->deprecated());  // Struct has no deprecated field.
		EncodeStructFieldToBuf(*pField, luaTable, pBuf);
		if (Bad()) return;
	}  // for
}  // EncodeStructToBuf()

void StructEncoder::CheckLuaTable(const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	const auto& vFields = *obj.fields();
	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField)
			ERR_RET("illegal field " + PopFullFieldName(sKey));
		assert(!pField->deprecated());
	}
}  // CheckLuaTable()

void StructEncoder::EncodeStructFieldToBuf(const Field& field,
	const LuaRef& luaTable, uint8_t* pBuf)
{
	assert(pBuf);
	const char* pFieldName = field.name()->c_str();
	const LuaRef luaValue = luaTable.get(pFieldName);
	if (!luaValue)
		ERR_RET("missing struct field " + PopFullFieldName(pFieldName));

	const reflection::Type& type = *field.type();
	// Todo: check type of value...

	uint16_t offset = field.offset();
	uint8_t* pDest = pBuf + offset;
	reflection::BaseType eBaseType = type.base_type();
	if (eBaseType <= reflection::Double)
	{
		EncodeScalarToBuf(eBaseType, luaValue, pBuf + offset);  // XXX throw?
		return;
	}

	assert(eBaseType == reflection::Obj);
	const Object* pFieldObj = Objects()[type.index()];
	assert(pFieldObj);
	assert(pFieldObj->is_struct());
	PushName(pFieldName);
	EncodeStructToBuf(*pFieldObj, luaValue, pDest);
	SafePopName();
}  // EncodeStructFieldToBuf()

template <typename T>
static void CopyToBuf(const LuaIntf::LuaRef& luaValue, uint8_t* pDest)
{
	T val = luaValue.toValue<T>();  // Todo: throw?
	*reinterpret_cast<T*>(pDest) = val;
}

// Encode struct scalar element to buffer.
void StructEncoder::EncodeScalarToBuf(reflection::BaseType eType,
	const LuaRef& luaValue, uint8_t* pDest)
{
	using namespace reflection;
	assert(pDest);
	assert(eType <= Double);
	switch (eType)
	{
	case UType:
	case Bool:
	case UByte:
		CopyToBuf<uint8_t>(luaValue, pDest);
		break;
	case Byte:
		CopyToBuf<int8_t>(luaValue, pDest);
		break;
	case Short:
		CopyToBuf<int16_t>(luaValue, pDest);
		break;
	case UShort:
		CopyToBuf<uint16_t>(luaValue, pDest);
		break;
	case Int:
		CopyToBuf<int32_t>(luaValue, pDest);
		break;
	case UInt:
		CopyToBuf<uint32_t>(luaValue, pDest);
		break;
	case Long:
		CopyToBuf<int64_t>(luaValue, pDest);
		break;
	case ULong:
		CopyToBuf<uint64_t>(luaValue, pDest);
		break;
	case Float:
		CopyToBuf<float>(luaValue, pDest);
		break;
	case Double:
		CopyToBuf<double>(luaValue, pDest);
		break;
	}  // switch
	assert(!"Illegal type.");
}  // EncodeScalarToBuf()
