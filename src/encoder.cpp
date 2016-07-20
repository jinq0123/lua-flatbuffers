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
	m_fbb.Clear();
	NameStack emptyStack;
	m_nameStack.swap(emptyStack);

	return Build(sName, table);
}

// Todo: check required fields.
// Todo: Skip default value.

std::tuple<bool, std::string>
Encoder::Build(const std::string& sName, const LuaIntf::LuaRef& table)
{
	m_nameStack.push(sName);
	const reflection::Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	const auto& vFields = *pObj->fields();

	for (auto& e : table)
	{
		std::string sKey = e.key<std::string>();
		const reflection::Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField)
		{
			return std::make_tuple(false,
				"illegal field " + GetFullFieldName(sKey));
		}
		if (pField->deprecated())
		{
			return std::make_tuple(false,
				"deprecated field " + GetFullFieldName(sKey));
		}

		LuaIntf::LuaRef value = e.value<LuaIntf::LuaRef>();
	}

	m_nameStack.pop();
	// XXX
	return std::make_tuple(false, "to be implemented");
}

std::string Encoder::GetFullFieldName(const std::string& sFieldName) const
{
	NameStack ns(m_nameStack);
	std::string sResult(sFieldName);
	while (!ns.empty())
	{
		sResult.insert(0, ns.top() + ".");
		ns.pop();
	}
	return sResult;
}

