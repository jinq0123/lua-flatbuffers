#include "decoder.h"

#include <LuaIntf/LuaIntf.h>

#include <flatbuffers/reflection.h>

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

	// Todo: verify buffer...
	m_pVerifier = std::make_unique<flatbuffers::Verifier>(
		reinterpret_cast<const uint8_t *>(pBuf), buf.size());
	m_isBufferIllegal = false;

	// Check the first offset field before GetRoot().
	if (!m_pVerifier->Verify<flatbuffers::uoffset_t>(pBuf))
		return std::make_tuple(Nil(), "buffer is too short");

	const Table* pRoot = flatbuffers::GetRoot<Table>(pBuf);
	assert(pRoot);
	const reflection::Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	LuaRef luaTable = DecodeObject(*pObj, *pRoot);
	if (m_isBufferIllegal)
		return std::make_tuple(Nil(), "illegal buffer");
	return std::make_tuple(luaTable, "");
}

void Decoder::SetLuaTableField(
	const Table& fbTable,
	const reflection::Field& field,
	LuaRef& rLuaTable)
{
	if (field.deprecated()) return;
	const reflection::Type& type = *field.type();
	const char* pName = field.name()->c_str();
	assert(pName);
	uint16_t offset = field.offset();

	if (field.required() && !fbTable.VerifyFieldRequired<
		flatbuffers::uoffset_t>(*m_pVerifier, offset))
		goto set_illegal;

	using flatbuffers::GetFieldI;
	using flatbuffers::GetFieldF;
	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		if (fbTable.VerifyField<uint8_t>(*m_pVerifier, offset)) goto set_illegal;
		rLuaTable[pName] = GetFieldI<uint8_t>(fbTable, field);
		break;
	case reflection::Byte:
		rLuaTable[pName] = GetFieldI<int8_t>(fbTable, field);
		break;
	case reflection::Short:
		rLuaTable[pName] = GetFieldI<int16_t>(fbTable, field);
		break;
	case reflection::UShort:
		rLuaTable[pName] = GetFieldI<uint16_t>(fbTable, field);
		break;
	case reflection::Int:
		rLuaTable[pName] = GetFieldI<int32_t>(fbTable, field);
		break;
	case reflection::UInt:
		rLuaTable[pName] = GetFieldI<uint32_t>(fbTable, field);
		break;
	case reflection::Long:
		rLuaTable[pName] = GetFieldI<int64_t>(fbTable, field);
		break;
	case reflection::ULong:
		rLuaTable[pName] = GetFieldI<uint64_t>(fbTable, field);
		break;
	case reflection::Float:
		rLuaTable[pName] = GetFieldF<float>(fbTable, field);
		break;
	case reflection::Double:
		rLuaTable[pName] = GetFieldF<double>(fbTable, field);
		break;

	case reflection::String:
	{
		const auto* pStr = fbTable.GetPointer<
			const flatbuffers::String *>(field.offset());
		if (pStr) rLuaTable[pName] = pStr->str();
		break;
	}
	case reflection::Vector:
		rLuaTable[pName] = DecodeVectorField(fbTable, field);
		break;
	case reflection::Obj:
	{
		const auto* pTable = fbTable.GetPointer<
			const flatbuffers::Table*>(field.offset());
		if (!pTable) break;
		rLuaTable[pName] = DecodeObject(*m_vObjects[type.index()], *pTable);
		break;
	}
	case reflection::Union:
		rLuaTable[pName] = DecodeUnionField(fbTable, field);
		break;
	default:
		assert(false);
		break;
	}
	return;

set_illegal:
	m_isBufferIllegal = true;
}

LuaRef Decoder::DecodeObject(
	const reflection::Object& object,
	const Table& fbTable)
{
	if (!fbTable.VerifyTableStart(*m_pVerifier))
		return SetIllegal();

	LuaRef luaTable = LuaRef::createTable(L);
	for (const reflection::Field* pField : *object.fields())
	{
		assert(pField);
		SetLuaTableField(fbTable, *pField, luaTable);
		if (m_isBufferIllegal) return Nil();
	}

	if (!m_pVerifier->EndTable())
		return SetIllegal();
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

LuaRef Decoder::Nil() const
{
	return LuaRef(L, nullptr);
}

LuaRef Decoder::SetIllegal()
{
	m_isBufferIllegal = true;
	return Nil();
}

