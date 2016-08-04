#include "struct_encoder.h"

flatbuffers::uoffset_t StructEncoder::EncodeStruct(
	const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());

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
	CheckLuaTable(obj, luaTable);
	if (Bad()) return;

	// Struct should traverse all fields of object.
	for (const Field* pField : *obj.fields())
	{
		assert(pField);
		assert(!pField->deprecated());  // Struct has no deprecated field.
		PushName(*pField);
		EncodeStructFieldToBuf(*pField, luaTable, pBuf);
		SafePopName();
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
		ERR_RET("missing struct field " + PopFullName());

	const reflection::Type& type = *field.type();
	// Todo: check type of value...

	uint16_t offset = field.offset();
	uint8_t* pDest = pBuf + offset;
	reflection::BaseType eBaseType = type.base_type();
	if (eBaseType <= reflection::Double)
	{
		EncodeScalarToBuf(eBaseType, luaValue, pBuf + offset);
		return;
	}

	assert(eBaseType == reflection::Obj);
	const Object* pFieldObj = Objects()[type.index()];
	assert(pFieldObj);
	assert(pFieldObj->is_struct());
	EncodeStructToBuf(*pFieldObj, luaValue, pDest);
}  // EncodeStructFieldToBuf()

template <typename T>
void StructEncoder::CopyScalarToBuf(
	const LuaIntf::LuaRef& luaValue, uint8_t* pDest)
{
	static_assert(std::is_scalar<T>::value,
		"CopyScalarToBuf() is only for scalar types.");
	T val = LuaToNumber<T>(luaValue);
	if (Bad()) return;
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
	case Bool:  // true -> 1
	case UByte: return CopyScalarToBuf<uint8_t>(luaValue, pDest);
	case Byte: return CopyScalarToBuf<int8_t>(luaValue, pDest);
	case Short: return CopyScalarToBuf<int16_t>(luaValue, pDest);
	case UShort: return CopyScalarToBuf<uint16_t>(luaValue, pDest);
	case Int: return CopyScalarToBuf<int32_t>(luaValue, pDest);
	case UInt: return CopyScalarToBuf<uint32_t>(luaValue, pDest);
	case Long: return CopyScalarToBuf<int64_t>(luaValue, pDest);
	case ULong: return CopyScalarToBuf<uint64_t>(luaValue, pDest);
	case Float: return CopyScalarToBuf<float>(luaValue, pDest);
	case Double: return CopyScalarToBuf<double>(luaValue, pDest);
	}  // switch
	assert(!"Illegal type.");
}  // EncodeScalarToBuf()
