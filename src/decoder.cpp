#include "decoder.h"

#include <LuaIntf/LuaIntf.h>

#include <flatbuffers/reflection.h>

#define ERR_RET_NIL(ErrorStr) do { \
	SetError(ErrorStr); \
	return Nil(); \
} while(0)

using LuaIntf::LuaRef;

Decoder::Decoder(lua_State* state, const reflection::Schema& schema) :
	L(state),
	m_schema(schema),
	m_vObjects(*schema.objects()),
	m_vEnums(*schema.enums())
{
	assert(L);
}

std::tuple<LuaRef, std::string>
Decoder::Decode(const std::string& sName, const std::string& buf)
{
	const char* pBuf = buf.data();

	m_pVerifier = std::make_unique<flatbuffers::Verifier>(
		reinterpret_cast<const uint8_t *>(pBuf), buf.size());
	m_sError.clear();

	// Check the first offset field before GetRoot().
	if (!m_pVerifier->Verify<flatbuffers::uoffset_t>(pBuf))
		return std::make_tuple(Nil(), "buffer is too short");

	const Table* pRoot = flatbuffers::GetRoot<Table>(pBuf);
	assert(pRoot);
	const reflection::Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	LuaRef luaTable = DecodeObject(*pObj, *pRoot);

	m_pVerifier.reset();
	return std::make_tuple(luaTable, m_sError);
}

LuaRef Decoder::DecodeFieldOfTable(
	const Table& fbTable, const reflection::Field& field)
{
	if (field.deprecated()) return Nil();
	if (field.required() && !fbTable.VerifyFieldRequired<
		flatbuffers::uoffset_t>(*m_pVerifier, field.offset()))
	{
		ERR_RET_NIL("illegal required field "
			+ PopFullFieldName(field.name()->c_str()));
	}

	reflection::BaseType eType = field.type()->base_type();
	if (eType <= reflection::Double)
		return DecodeScalarField(fbTable, field);

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

LuaRef Decoder::DecodeObjectField(
	const Table& fbTable, const reflection::Field& field)
{
	uint16_t offset = field.offset();
	if (!fbTable.VerifyField<flatbuffers::uoffset_t>(*m_pVerifier, offset))
	{
		ERR_RET_NIL("illegal offset to object field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	const auto* pTable = fbTable.GetPointer<const flatbuffers::Table*>(offset);
	if (!pTable) return Nil();
	return DecodeObject(*m_vObjects[field.type()->index()], *pTable);  // XXX verify
}

LuaRef Decoder::DecodeStringField(
	const Table& fbTable, const reflection::Field& field)
{
	uint16_t offset = field.offset();
	if (!fbTable.VerifyField<flatbuffers::uoffset_t>(*m_pVerifier, offset))
	{
		ERR_RET_NIL("illegal offset to string field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	const auto* pStr = fbTable.GetPointer<const flatbuffers::String *>(offset);
	if (!m_pVerifier->Verify(pStr))
	{
		ERR_RET_NIL("illegal string field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	if (pStr) return LuaRef::fromValue(L, pStr->str());
	return Nil();
}

LuaRef Decoder::DecodeScalarField(
	const Table& fbTable, const reflection::Field& field)
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

LuaRef Decoder::DecodeObject(
	const reflection::Object& object,
	const Table& fbTable)
{
	m_nameStack.push(object.name()->str());
	if (!fbTable.VerifyTableStart(*m_pVerifier))
		ERR_RET_NIL("illegal start of table " + PopFullName());

	LuaRef luaTable = LuaRef::createTable(L);
	for (const reflection::Field* pField : *object.fields())
	{
		assert(pField);
		const char* pName = pField->name()->c_str();
		assert(pName);
		luaTable[pName] = DecodeFieldOfTable(fbTable, *pField);
		if (Bad()) return Nil();
	}

	if (!m_pVerifier->EndTable())
		ERR_RET_NIL("illegal end of table " + PopFullName());

	m_nameStack.pop();
	return luaTable;
}

LuaRef Decoder::DecodeVectorField(
	const Table& table,
	const reflection::Field& field)
{
	const auto* pVec = table.GetPointer<
		const flatbuffers::VectorOfAny*>(field.offset());
	if (!pVec) return Nil();

	const reflection::Type& type = *field.type();
	return DecodeVector(type, *pVec);
}

LuaRef Decoder::DecodeVector(
	const reflection::Type& type,
	const flatbuffers::VectorOfAny& v)
{
	assert(reflection::Vector == type.base_type());
	reflection::BaseType elemType = type.element();

	// Todo: may be map (if has key)...
	// Todo: Move switch(elemType) out...

	LuaRef luaArray = LuaRef::createTable(L);
	const flatbuffers::VectorOfAny* pVec = &v;
	for (size_t i = 1; i <= v.size(); ++i)
	{
		switch (elemType)
		{
		case reflection::UType:
		case reflection::Bool:
		case reflection::UByte:
		case reflection::Byte:
		case reflection::Short:
		case reflection::UShort:
		case reflection::Int:
		case reflection::UInt:
		case reflection::Long:
			luaArray[i+1] = GetAnyVectorElemI(pVec, elemType, i);
			break;
		case reflection::ULong:
			luaArray[i+1] = static_cast<uint64_t>(
				GetAnyVectorElemI(pVec, elemType, i));
			break;
		case reflection::Float:
		case reflection::Double:
			luaArray[i+1] = GetAnyVectorElemF(pVec, elemType, i);
			break;
		case reflection::String:
			luaArray[i+1] = GetAnyVectorElemS(pVec, elemType, i);
			break;
		case reflection::Vector:
			assert(!"Nesting vectors is not supported.");
			break;
		case reflection::Obj:
		{
			const auto* pTable = flatbuffers::GetAnyVectorElemPointer<const Table>(pVec, i);
			// Todo: check pTable
			luaArray[i+1] = DecodeObject(*m_vObjects[type.index()], *pTable);
			break;
		}
		case reflection::Union:
			assert(!"Union must always be part of a table.");
			break;
		default:
			assert(false);
			break;
		}  // switch
	}  // for
	return luaArray;
}

LuaRef Decoder::DecodeUnionField(
	const Table& table,
	const reflection::Field& field)
{
	assert(!field.deprecated());
	const reflection::Type& type = *field.type();
	assert(type.base_type() == reflection::Union);
	const void* pVoid = table.GetPointer<const void*>(field.offset());
	if (!pVoid) return Nil();

	const reflection::Enum& e = *m_vEnums[type.index()];
	assert(e.is_union());
	const reflection::Type& underlyingType = *e.underlying_type();
	return Decode(underlyingType, pVoid);
}

template <typename T>
inline LuaRef ReadScalar(lua_State* L, const void* pVoid)
{
	assert(L && pVoid);
	using flatbuffers::ReadScalar;
	return LuaRef::fromValue(L, ReadScalar<T>(pVoid));
}

LuaRef Decoder::Decode(
	const reflection::Type& type,
	const void* pVoid)
{
	if (!pVoid) return Nil();

	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		return ReadScalar<uint8_t>(L, pVoid);
	case reflection::Byte:
		return ReadScalar<int8_t>(L, pVoid);
	case reflection::Short:
		return ReadScalar<int16_t>(L, pVoid);
	case reflection::UShort:
		return ReadScalar<uint16_t>(L, pVoid);
	case reflection::Int:
		return ReadScalar<uint8_t>(L, pVoid);
	case reflection::UInt:
		return ReadScalar<uint8_t>(L, pVoid);
	case reflection::Long:
		return ReadScalar<int64_t>(L, pVoid);
	case reflection::ULong:
		return ReadScalar<uint64_t>(L, pVoid);
	case reflection::Float:
		return ReadScalar<float>(L, pVoid);
	case reflection::Double:
		return ReadScalar<double>(L, pVoid);
	case reflection::String:
	{
		const auto* pStr = reinterpret_cast<const flatbuffers::String*>(pVoid);
		return LuaRef::fromValue(L, pStr->str());
	}
	case reflection::Vector:
	{
		const auto* pVec = reinterpret_cast<const flatbuffers::VectorOfAny*>(pVoid);
		return DecodeVector(type, *pVec);
	}
	case reflection::Obj:
	{
		const auto* pTable = reinterpret_cast<const Table*>(pVoid);
		return DecodeObject(*m_vObjects[type.index()], *pTable);
	}
	case reflection::Union:
	{
		const reflection::Enum& e = *m_vEnums[type.index()];
		assert(e.is_union());
		const reflection::Type& underlyingType = *e.underlying_type();
		return Decode(underlyingType, pVoid);
	}
	}  // switch

	assert(false);
	return Nil();
}

template<typename T>
LuaRef Decoder::DecodeFieldI(const Table& fbTable,
	const reflection::Field &field)
{
	if (fbTable.VerifyField<T>(*m_pVerifier, field.offset()))
	{
		ERR_RET_NIL("illegal int field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	T i = flatbuffers::GetFieldI<T>(fbTable, field);
	return LuaRef::fromValue(L, i);
}

template<typename T>
LuaRef Decoder::DecodeFieldF(const Table& fbTable,
	const reflection::Field &field)
{
	if (fbTable.VerifyField<T>(*m_pVerifier, field.offset()))
	{
		ERR_RET_NIL("illegal float field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	T f = flatbuffers::GetFieldF<T>(fbTable, field);
	return LuaRef::fromValue(L, f);
}

LuaRef Decoder::Nil() const
{
	return LuaRef(L, nullptr);
}

void Decoder::SetError(const std::string& sError)
{
	m_sError = sError;
}

std::string Decoder::PopFullName()
{
	return m_nameStack.PopFullName();
}

std::string Decoder::PopFullFieldName(const std::string& sFieldName)
{
	return m_nameStack.PopFullFieldName(sFieldName);
}

