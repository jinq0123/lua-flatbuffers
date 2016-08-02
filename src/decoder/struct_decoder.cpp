#include "struct_decoder.h"

using LuaIntf::LuaRef;

LuaRef StructDecoder::DecodeStruct(const reflection::Object& object,
	const flatbuffers::Struct& fbStruct)
{
	assert(object.is_struct());
	PushName(object);
	if (!Verifier().Verify(&fbStruct, object.bytesize()))
		ERR_RET_NIL("illegal struct " + PopFullName());

	LuaRef luaTable = CreateLuaTable();
	for (const reflection::Field* pField : *object.fields())
	{
		assert(pField);
		const char* pName = pField->name()->c_str();
		assert(pName);
		luaTable[pName] = DecodeFieldOfStruct(fbStruct, *pField);
		if (Bad()) return Nil();
	}

	SafePopName();
	return luaTable;
}

LuaRef StructDecoder::DecodeFieldOfStruct(
	const Struct& fbStruct, const reflection::Field& field)
{
	if (field.deprecated()) return Nil();
	reflection::BaseType eType = field.type()->base_type();
	if (eType <= reflection::Double)
		return DecodeScalarField(fbStruct, field);

	assert(eType == reflection::Obj);
	return DecodeObjectField(fbStruct, field);
}

LuaRef StructDecoder::DecodeScalarField(
	const Struct& fbStruct, const reflection::Field& field)
{
	reflection::BaseType eType = field.type()->base_type();
	if (eType <= reflection::ULong)
	{
		int64_t l = flatbuffers::GetAnyFieldI(fbStruct, field);
		if (eType != reflection::ULong)
			return LuaRef::fromValue(LuaState(), l);
		return LuaRef::fromValue(LuaState(), static_cast<uint64_t>(l));
	}
	assert(eType <= reflection::Double);
	double d = flatbuffers::GetAnyFieldF(fbStruct, field);
	return LuaRef::fromValue(LuaState(), d);
}

LuaRef StructDecoder::DecodeObjectField(
	const Struct& fbStruct, const reflection::Field& field)
{
	const reflection::Object* pObj = Objects()[field.type()->index()];
	assert(pObj);
	assert(pObj->is_struct());

	const uint8_t* pData = fbStruct.GetAddressOf(field.offset());
	return StructDecoder(m_rCtx).DecodeStruct(*pObj,
		*reinterpret_cast<const flatbuffers::Struct*>(pData));
}
