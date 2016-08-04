#include "union_encoder.h"

#include "table_encoder.h"  // for TableEncoder

#include <sstream>  // for ostringstream

using reflection::EnumVal;

flatbuffers::uoffset_t UnionEncoder::EncodeUnion(
	const Enum& enu,
	const LuaRef& luaType,
	const LuaRef& luaValue)
{
	const EnumVal* pEnumVal = GetEnumVal(enu, luaType);
	if (Bad()) return 0;
	assert(pEnumVal);

	const reflection::Object* pObj = pEnumVal->object();
	assert(pObj);
	// only tables can be union elements
	assert(!pObj->is_struct());
	return TableEncoder(m_rCtx).EncodeTable(*pObj, luaValue);
}

// Return null if error.
const EnumVal* UnionEncoder::GetEnumVal(
	const Enum& enu, const LuaRef& luaType)
{
	if (!luaType)
	{
		SetError("missing union type field " + PopFullUnionTypeName());
		return nullptr;
	}

	using LuaIntf::LuaTypeID;
	LuaTypeID luaTypeId = luaType.type();
	if (LuaTypeID::NUMBER == luaTypeId)
	{
		int64_t qwType = LuaToNumber<int64_t>(luaType);
		if (Bad()) return nullptr;
		return GetEnumValFromNum(enu, qwType);
	}

	if (LuaTypeID::STRING == luaTypeId)
	{
		string sType = luaType.toValue<string>();
		return GetEnumValFromName(enu, sType);
	}

	SetError("union type " + PopFullUnionTypeName() + " is " + luaType.typeName());
	return nullptr;
}

const EnumVal* UnionEncoder::GetEnumValFromNum(
	const Enum& enu, int64_t qwType)
{
	const EnumVal* pEnumVal = enu.values()->LookupByKey(qwType);
	if (pEnumVal) return pEnumVal;
	std::ostringstream oss;
	oss << "illegal union type " << PopFullUnionTypeName()
		<< "(" << qwType << ")";
	SetError(oss.str());
	return nullptr;
}

const EnumVal* UnionEncoder::GetEnumValFromName(
	const Enum& enu, const std::string& sType)
{
	for (const EnumVal* pEnumVal : *enu.values())
	{
		assert(pEnumVal);
		if (pEnumVal->name()->c_str() == sType)
			return pEnumVal;
	}
	SetError("illegal union type name " + PopFullUnionTypeName()
		+ "(" + sType + ")");
	return nullptr;
}

std::string UnionEncoder::PopFullUnionTypeName()
{
	return PopFullName() + flatbuffers::UnionTypeFieldSuffix();
}

