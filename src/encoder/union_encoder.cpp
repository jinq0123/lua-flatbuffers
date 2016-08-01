#include "union_encoder.h"

flatbuffers::uoffset_t UnionEncoder::EncodeUnion(
	const reflection::Enum& enu,
	const LuaRef& luaType,
	const LuaRef& luaValue)
{
	m_pEnum = &enu;

	int64_t qwType = GetType(luaType);
	if (Bad()) return 0;

	// XXX
	return 0;
}

int64_t UnionEncoder::GetType(const LuaRef& luaType)
{
	if (!luaType)
	{
		SetError("missing type of union " + PopFullName());
		return -1;
	}

	try
	{
		return luaType.toValue<int64_t>();
	}
	catch (...)
	{
	}

	string sType = luaType.toValue<string>();
	return GetTypeFromName(sType);
}

int64_t UnionEncoder::GetTypeFromName(const string& sType)
{
	for (const reflection::EnumVal* pEnumVal : *m_pEnum->values())
	{
		assert(pEnumVal);
		if (pEnumVal->name()->c_str() == sType)
			return pEnumVal->value();
	}
	SetError("illegal union " + PopFullName() + "'s type '" + sType + "'");
	return -1;
}

