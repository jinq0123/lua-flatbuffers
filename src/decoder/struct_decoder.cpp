#include "struct_decoder.h"

#include <flatbuffers/reflection.h>

using LuaIntf::LuaRef;

StructDecoder::StructDecoder(DecoderContext& rCtx) : DecoderBase(rCtx)
{
}

LuaRef StructDecoder::DecodeStruct(const reflection::Object& object,
	const flatbuffers::Struct& fbStruct)
{
	assert(object.is_struct());
	PushName(object.name()->str());
	if (Verifier().Verify(&fbStruct, object.bytesize()))
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
		return DecodeScalarField(fbTable, field);

	if (!VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field))
		return Nil();

	switch (eType)
	{
	case reflection::String:
		return DecodeStringField(fbTable, field);
	case reflection::Vector:
		return DecodeVectorField(fbTable, field);
	case reflection::Obj:
		return DecodeObjectField(fbTable, field);
	case reflection::Union:
		return DecodeUnionField(fbTable, field);
	}
	assert(!"Illegal field type.");
	return Nil();
}

LuaRef StructDecoder::DecodeScalarField(
	const Struct& fbStruct, const reflection::Field& field)
{
	switch (field.type()->base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		return DecodeFieldI<uint8_t>(fbTable, field);
	case reflection::Byte:
		return DecodeFieldI<int8_t>(fbTable, field);
	case reflection::Short:
		return DecodeFieldI<int16_t>(fbTable, field);
	case reflection::UShort:
		return DecodeFieldI<uint16_t>(fbTable, field);
	case reflection::Int:
		return DecodeFieldI<int32_t>(fbTable, field);
	case reflection::UInt:
		return DecodeFieldI<uint32_t>(fbTable, field);
	case reflection::Long:
		return DecodeFieldI<int64_t>(fbTable, field);
	case reflection::ULong:
		return DecodeFieldI<uint64_t>(fbTable, field);
	case reflection::Float:
		return DecodeFieldF<float>(fbTable, field);
	case reflection::Double:
		return DecodeFieldF<double>(fbTable, field);
	}
	assert(!"Illegal scalar field type.");
	return Nil();
}

LuaRef StructDecoder::DecodeStringField(
	const Struct& fbStruct, const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const flatbuffers::String* pStr = flatbuffers::GetFieldS(fbTable, field);
	if (!Verifier().Verify(pStr))
	{
		ERR_RET_NIL("illegal string field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	if (pStr) return LuaRef::fromValue(LuaState(), pStr->str());
	return Nil();
}

LuaRef StructDecoder::DecodeVectorField(
	const Struct& fbStruct,
	const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const auto* pVec = fbTable.GetPointer<
		const flatbuffers::VectorOfAny*>(field.offset());
	if (!pVec) return Nil();

	const reflection::Type& type = *field.type();
	PushName(field.name()->c_str());
	LuaRef luaTable = DecodeVector(type, *pVec);
	SafePopName();
	return luaTable;
}

LuaRef StructDecoder::DecodeObjectField(
	const Struct& fbStruct, const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const void* pData = fbTable.GetPointer<const void*>(field.offset());
	return DecodeObject(*Objects()[field.type()->index()], pData);
}

LuaRef StructDecoder::DecodeUnionField(const Struct& fbStruct,
	const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const reflection::Type& type = *field.type();
	assert(type.base_type() == reflection::Union);
	const void* pData = fbTable.GetPointer<const void*>(field.offset());
	if (!pData) return Nil();

	const reflection::Enum& e = *(*m_rCtx.schema.enums())[type.index()];
	assert(e.is_union());
	const reflection::Type& underlyingType = *e.underlying_type();
	PushName(field.name()->c_str());
	LuaRef luaRef = DecodeUnion(underlyingType, pData);
	SafePopName();
	return luaRef;
}

template<typename T>
LuaRef StructDecoder::DecodeFieldI(const Struct& fbStruct,
	const reflection::Field &field)
{
	if (!VerifyFieldOfTable<T>(fbTable, field))
		return Nil();
	T i = flatbuffers::GetFieldI<T>(fbTable, field);
	return LuaRef::fromValue(LuaState(), i);
}

template<typename T>
LuaRef StructDecoder::DecodeFieldF(const Struct& fbStruct,
	const reflection::Field &field)
{
	if (!VerifyFieldOfTable<T>(fbTable, field))
		return Nil();
	T f = flatbuffers::GetFieldF<T>(fbTable, field);
	return LuaRef::fromValue(LuaState(), f);
}

