#include "decoder.h"

#include <LuaIntf/LuaIntf.h>

#include <flatbuffers/reflection.h>

Decoder::Decoder(lua_State* state, const reflection::Schema& schema) :
	L(state),
	m_schema(schema),
	m_vObjects(*schema.objects())
{
	assert(L);
}

void Decoder::SetLuaTableField(
	const Table& fbTable,
	const reflection::Field& field,
	LuaRef& rLuaTable) const
{
	if (field.deprecated()) return;
	const reflection::Type& type = *field.type();
	const char* pName = field.name()->c_str();
	assert(pName);
	uint16_t offset = field.offset();

	using flatbuffers::GetFieldI;
	using flatbuffers::GetFieldF;
	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
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
		// XXX
		break;
	case reflection::Union:
		// XXX
		break;
	default:
		assert(false);
		break;
	}
}

std::tuple<LuaIntf::LuaRef, std::string>
Decoder::Decode(const std::string& sName, const std::string& buf) const
{
	const Table* pRoot = flatbuffers::GetRoot<Table>(buf.data());
	assert(pRoot);

	// Todo: verify buffer...

	const reflection::Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	return std::make_tuple(DecodeObject(*pObj, *pRoot), "");
	// Todo: return error.
}

LuaIntf::LuaRef Decoder::DecodeObject(
	const reflection::Object& object,
	const Table& fbTable) const
{
	LuaRef luaTable = LuaRef::createTable(L);
	for (const reflection::Field* pField : *object.fields())
	{
		assert(pField);
		SetLuaTableField(fbTable, *pField, luaTable);
	}

	return luaTable;
}

LuaIntf::LuaRef Decoder::DecodeVectorField(
	const Table& table,
	const reflection::Field& field) const
{
	const auto* pVec = table.GetPointer<
		const flatbuffers::VectorOfAny*>(field.offset());
	if (!pVec) return LuaRef(L, nullptr);

	const reflection::Type& type = *field.type();
	assert(reflection::Vector == type.base_type());
	reflection::BaseType elemType = type.element();

	// Todo: may be map (if has key)...

	LuaRef luaArray = LuaRef::createTable(L);
	for (size_t i = 1; i <= pVec->size(); ++i)
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
			assert(false);
			break;
		case reflection::Obj:
			// XXX
			break;
		case reflection::Union:
			// XXX
			break;
		default:
			assert(false);
			break;
		}  // switch
	}  // for
	return luaArray;
}
