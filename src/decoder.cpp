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
		if (pField->deprecated()) continue;
		const reflection::Type& type = *pField->type();
		const char* pName = pField->name()->c_str();
		assert(pName);
		switch (type.base_type())
		{
		case reflection::UType:
		case reflection::Bool:
		case reflection::UByte:
			luaTable[pField->name()] = "XXX";
			AddElement<uint8_t>(offset, elementValue, defInt);
			break;
		case reflection::Byte:
			AddElement<int8_t>(offset, elementValue, defInt);
			break;
		case reflection::Short:
			AddElement<int16_t>(offset, elementValue, defInt);
			break;
		case reflection::UShort:
			AddElement<uint16_t>(offset, elementValue, defInt);
			break;
		case reflection::Int:
			AddElement<int32_t>(offset, elementValue, defInt);
			break;
		case reflection::UInt:
			AddElement<uint32_t>(offset, elementValue, defInt);
			break;
		case reflection::Long:
			AddElement<int64_t>(offset, elementValue, defInt);
			break;
		case reflection::ULong:
			AddElement<uint64_t>(offset, elementValue, defInt);
			break;
		case reflection::Float:
			AddElement<float>(offset, elementValue, defReal);
			break;
		case reflection::Double:
			AddElement<double>(offset, elementValue, defReal);
			break;

		case reflection::String:
			// XXX
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
			table[pField->name()] = "XXX";
			break;
		}
	}
	// XXX
	return std::make_tuple(table, "");
}

