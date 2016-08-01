#include "union_encoder.h"

flatbuffers::uoffset_t UnionEncoder::EncodeUnion(
	const reflection::Enum& enu,
	const LuaRef& luaType,
	const LuaRef& luaValue)
{
	m_pEnum = &enu;

	int64_t qwType = GetType(luaType);
	if (Bad()) return 0;

	const reflection::EnumVal* pEnumVal = enu.values()->LookupByKey(qwType);
	if (!pEnumVal)
	{
		SetError("illegal union " + PopFullName() + "'s type "
			+ luaType.toValue<string>());
		return 0;
	}
	// XXX

	return 0;
}

// Return null if error.
const reflection::EnumVal* UnionEncoder::GetEnumVal(
	const reflection::Enum& enu, const LuaRef& luaType)
{
	if (!luaType)
	{
		SetError("missing type of union " + PopFullName());
		return nullptr;
	}
	LuaTypeID luaTypeId = luaType.type();
	if (LuaTypeID::NUMBER == luaTypeId)
	{
		int64_t qwType = luaType.toValue<int64_t>();
		return GetEnumValFromNum(qwType);
		//const reflection::EnumVal* pEnumVal = m_pEnum->LookupByKey(qwType);
		//if (!pEnumVal)
		//	SetError("illegal union " + PopFullName() " "'s type " + )

	}
	if (LuaTypeID::STRING == luaTypeId)
	{
		string sType = luaType.toValue<string>();
		return GetEnumValFromName(sType);
	}
	SetError("union " + PopFullName()
		+ "'s type must be number or string but not " + luaType.typeName());
	return nullptr;
}

int64_t UnionEncoder::GetType(const LuaRef& luaType)
{


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

