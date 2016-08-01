#include "table_decoder.h"

#include "object_decoder.h"  // for ObjectDecoder
#include "union_decoder.h"  // for UnionDecoder
#include "vector_decoder.h"  // for VectorDecoder

using LuaIntf::LuaRef;

LuaRef TableDecoder::DecodeFieldOfTable(
	const Table& fbTable, const Field& field)
{
	using namespace reflection;
	if (field.deprecated()) return Nil();
	BaseType eType = field.type()->base_type();
	if (eType <= Double)
		return DecodeScalarField(fbTable, field);

	if (!VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field))
		return Nil();

	switch (eType)
	{
	case String:
		return DecodeStringField(fbTable, field);
	case Vector:
		return DecodeVectorField(fbTable, field);
	case Obj:
		return DecodeObjectField(fbTable, field);
	case Union:
		return DecodeUnionField(fbTable, field);
	}
	assert(!"Illegal field type.");
	return Nil();
}

LuaRef TableDecoder::DecodeScalarField(
	const Table& fbTable, const Field& field)
{
	using namespace reflection;
	switch (field.type()->base_type())
	{
	case UType:
	case Bool:
	case UByte:
		return DecodeFieldI<uint8_t>(fbTable, field);
	case Byte:
		return DecodeFieldI<int8_t>(fbTable, field);
	case Short:
		return DecodeFieldI<int16_t>(fbTable, field);
	case UShort:
		return DecodeFieldI<uint16_t>(fbTable, field);
	case Int:
		return DecodeFieldI<int32_t>(fbTable, field);
	case UInt:
		return DecodeFieldI<uint32_t>(fbTable, field);
	case Long:
		return DecodeFieldI<int64_t>(fbTable, field);
	case ULong:
		return DecodeFieldI<uint64_t>(fbTable, field);
	case Float:
		return DecodeFieldF<float>(fbTable, field);
	case Double:
		return DecodeFieldF<double>(fbTable, field);
	}
	assert(!"Illegal scalar field type.");
	return Nil();
}

LuaRef TableDecoder::DecodeStringField(
	const Table& fbTable, const Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const flatbuffers::String* pStr = flatbuffers::GetFieldS(fbTable, field);
	if (!pStr) return Nil();
	if (!Verifier().Verify(pStr))
		ERR_RET_NIL("illegal string field " + PopFullFieldName(field));
	return LuaRef::fromValue(LuaState(), pStr->str());
}

LuaRef TableDecoder::DecodeVectorField(
	const Table& fbTable, const Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const auto* pVec = fbTable.GetPointer<
		const flatbuffers::VectorOfAny*>(field.offset());
	if (!pVec) return Nil();

	const reflection::Type& type = *field.type();
	PushName(field);
	LuaRef luaTable = VectorDecoder(m_rCtx).DecodeVector(type, *pVec);
	SafePopName();
	return luaTable;
}

LuaRef TableDecoder::DecodeObjectField(
	const Table& fbTable, const Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const void* pData = fbTable.GetPointer<const void*>(field.offset());
	return ObjectDecoder(m_rCtx).DecodeObject(
		*Objects()[field.type()->index()], pData);
}

LuaRef TableDecoder::DecodeUnionField(
	const Table& fbTable, const Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const reflection::Type& type = *field.type();
	assert(type.base_type() == reflection::Union);
	const void* pData = fbTable.GetPointer<const void*>(field.offset());
	if (!pData) return Nil();

	const reflection::Enum& e = *(*m_rCtx.schema.enums())[type.index()];
	assert(e.is_union());
	// Enum underlying type: Byte Short or Int...
	const reflection::Type& underlyingType = *e.underlying_type();
	PushName(field);
	// XXX Get union real type...
	LuaRef luaRef = UnionDecoder(m_rCtx).DecodeUnion(underlyingType, pData);
	SafePopName();
	return luaRef;
}

template<typename T>
LuaRef TableDecoder::DecodeFieldI(const Table& fbTable, const Field &field)
{
	if (!VerifyFieldOfTable<T>(fbTable, field))
		return Nil();
	T i = flatbuffers::GetFieldI<T>(fbTable, field);
	return LuaRef::fromValue(LuaState(), i);
}

template<typename T>
LuaRef TableDecoder::DecodeFieldF(const Table& fbTable, const Field &field)
{
	if (!VerifyFieldOfTable<T>(fbTable, field))
		return Nil();
	T f = flatbuffers::GetFieldF<T>(fbTable, field);
	return LuaRef::fromValue(LuaState(), f);
}

LuaRef TableDecoder::DecodeTable(
	const reflection::Object& object, const Table& fbTable)
{
	assert(!object.is_struct());
	PushName(object);
	if (!fbTable.VerifyTableStart(Verifier()))
		ERR_RET_NIL("illegal start of table " + PopFullName());

	LuaRef luaTable = CreateLuaTable();
	for (const Field* pField : *object.fields())
	{
		assert(pField);
		const char* pName = pField->name()->c_str();
		assert(pName);
		luaTable[pName] = DecodeFieldOfTable(fbTable, *pField);
		if (Bad()) return Nil();
	}

	if (!Verifier().EndTable())
		ERR_RET_NIL("illegal end of table " + PopFullName());

	SafePopName();
	return luaTable;
}

template <typename T>
bool TableDecoder::VerifyFieldOfTable(const Table& fbTable, const Field &field)
{
	static_assert(std::is_scalar<T>::value, "T must be a scalar type");

	if (field.required())
	{
		if (fbTable.VerifyFieldRequired<T>(Verifier(), field.offset()))
			return true;

		SetError("illegal required field " + PopFullFieldName(field));
		return false;
	}

	if (fbTable.VerifyField<T>(Verifier(), field.offset()))
		return true;

	SetError("illegal offset of field " + PopFullFieldName(field));
	return false;
}
