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
	Reset();

	if (!EncodeToFbb(sName, table))  // An offset of 0 means error.
		return std::make_tuple(false, m_sError);

	const char* pBuffer = reinterpret_cast<const char*>(
		m_fbb.GetBufferPointer());
	return std::make_tuple(true, std::string(pBuffer, m_fbb.GetSize()));
}

// Todo: check required fields.
// Todo: Skip default value.

flatbuffers::uoffset_t
Encoder::EncodeToFbb(const std::string& sName, const LuaIntf::LuaRef& table)
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
			m_sError = "illegal field " + GetFullFieldName(sKey);
			return 0;
		}
		if (pField->deprecated())
		{
			m_sError = "deprecated field " + GetFullFieldName(sKey);
			return 0;
		}

		LuaIntf::LuaRef value = e.value<LuaIntf::LuaRef>();
	}

	m_nameStack.pop();
	// XXX
	m_sError = "to be implemented";
	return 0;
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

void Encoder::Reset()
{
	m_fbb.Clear();
	m_sError.clear();

	NameStack emptyStack;
	m_nameStack.swap(emptyStack);  // swap to clear
}

