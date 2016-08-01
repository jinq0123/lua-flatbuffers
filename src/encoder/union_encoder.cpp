#include "union_encoder.h"

#include <sstream>  // for ostringstream

flatbuffers::uoffset_t UnionEncoder::EncodeUnion(
	const reflection::Enum& enu,
	const LuaRef& luaType,
	const LuaRef& luaValue)
{
	const reflection::EnumVal* pEnumVal = GetEnumVal(enu, luaType);
	if (Bad()) return 0;
	assert(pEnumVal);

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

	using LuaIntf::LuaTypeID;
	LuaTypeID luaTypeId = luaType.type();
	if (LuaTypeID::NUMBER == luaTypeId)
	{
		int64_t qwType = luaType.toValue<int64_t>();
		return GetEnumValFromNum(enu, qwType);
	}

	if (LuaTypeID::STRING == luaTypeId)
	{
		string sType = luaType.toValue<string>();
		return GetEnumValFromName(enu, sType);
	}

	SetError("union " + PopFullName() + "'s type is " + luaType.typeName());
	return nullptr;
}

const reflection::EnumVal* UnionEncoder::GetEnumValFromNum(
	const reflection::Enum& enu, int64_t qwType)
{
	const reflection::EnumVal* pEnumVal = enu.values()->LookupByKey(qwType);
	if (pEnumVal) return pEnumVal;
	std::ostringstream oss;
	oss << "illegal union " << PopFullName() << "'s type value " << qwType;
	SetError(oss.str());
	return nullptr;
}

const reflection::EnumVal* UnionEncoder::GetEnumValFromName(
	const reflection::Enum& enu, const std::string& sType)
{
	for (const reflection::EnumVal* pEnumVal : *enu.values())
	{
		assert(pEnumVal);
		if (pEnumVal->name()->c_str() == sType)
			return pEnumVal;
	}
	SetError("illegal union " + PopFullName() + "'s type '" + sType + "'");
	return nullptr;
}

