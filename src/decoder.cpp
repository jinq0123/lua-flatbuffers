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

template<typename FieldType>
inline FieldType GetField(const flatbuffers::Table& fbTable,
	const reflection::Field& field)
{
	return fbTable.GetField<FieldType>(field.offset(),
		static_cast<FieldType>(fdefaultVal));
}

static void SetField(const flatbuffers::Table& fbTable,
	const reflection::Field& field,
	LuaIntf::LuaRef& rLuaTable)
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
		rLuaTable[pName] = fbTable.GetPointer<const flatbuffers::String *>(
			field.offset())->str();
		break;
	case reflection::Vector:
		// XXX
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
Decoder::Decode(const std::string& sName, const std::string& buf)
{
	// Todo: verify buffer...

	const reflection::Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);

	const flatbuffers::Table& fbTable =
		*flatbuffers::GetRoot<flatbuffers::Table>(buf.data());

	LuaRef luaTable = LuaRef::createTable(L);
	for (const reflection::Field* pField : *pObj->fields())
	{
		assert(pField);
		SetField(fbTable, *pField, luaTable);
	}

	// XXX
	return std::make_tuple(luaTable, "");
}

