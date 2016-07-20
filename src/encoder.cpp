#include "encoder.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

#include <LuaIntf/LuaIntf.h>

Encoder::Encoder(const reflection::Schema& schema) :
	m_schema(schema),
	m_vObjects(*schema.objects())
{
}

std::tuple<bool, std::string>
Encoder::Encode(const std::string& sName, const LuaIntf::LuaRef& table)
{
	const reflection::Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	const auto& vFields = *pObj->fields();

	for (auto& e : table)
	{
		std::string sKey = e.key<std::string>();
		const reflection::Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField) return std::make_tuple(false, "illegal field " + sKey);

		LuaIntf::LuaRef value = e.value<LuaIntf::LuaRef>();
	}

	// XXX
	return std::make_tuple(false, "to be implemented");
}

