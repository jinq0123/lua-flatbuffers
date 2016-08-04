#include "encoder_base.h"

#include "name_stack.h"  // for PopFullName()

#include <sstream>  // for ostringstream

void EncoderBase::SetError(const string& sError)
{
	m_rCtx.sError = sError;
}

std::string EncoderBase::PopFullName()
{
	return m_rCtx.nameStack.PopFullName() + GetVectorIndex();
}

std::string EncoderBase::PopFullFieldName(const string& sFieldName)
{
	return m_rCtx.nameStack.PopFullFieldName(sFieldName) + GetVectorIndex();
}

std::string EncoderBase::PopFullFieldName(const reflection::Field& field)
{
	return PopFullFieldName(field.name()->c_str());
}

int64_t EncoderBase::LuaToInt64(const LuaRef& luaValue)
{
	LuaIntf::LuaTypeID luaTypeId = luaValue.type();
	if (LuaIntf::LuaTypeID::BOOLEAN == luaTypeId)
		return luaValue.toValue<bool>() ? 1 : 0;

	// Directly toValue<int64_t>() may throw.
	// And toValue<double>() is wrong for 9223372036854775807.1 (maxinteger + 0.1).
	LuaRef toInt(luaValue.state(), "math.tointeger");
	LuaRef luaInt = toInt.call<LuaRef>(luaValue);
	if (luaInt) return luaInt.toValue<int64_t>();
	SetLuaToNumError(luaValue, true/* bToInt */);
	return 0;
}

template <> float EncoderBase::LuaToNumber<float>(const LuaRef& luaValue)
{
	double d = LuaToNumber<double>(luaValue);
	if (Bad()) return 0.0;
	return static_cast<float>(d);
}

template <> double EncoderBase::LuaToNumber<double>(const LuaRef& luaValue)
{
	// Allow string convert to number.
	LuaRef toNum(luaValue.state(), "tonumber");
	LuaRef luaNum = toNum.call<LuaRef>(luaValue);
	if (luaNum) return luaNum.toValue<double>();  // allow int64 -> double
	SetLuaToNumError(luaValue, false/* bToInt */);
	return 0.0;
}

void EncoderBase::SetLuaToNumError(const LuaRef& luaValue, bool bToInt)
{
	using LuaIntf::LuaTypeID;
	LuaTypeID luaTypeId = luaValue.type();
	std::string sValue;
	if (luaTypeId == LuaTypeID::STRING || luaTypeId == LuaTypeID::NUMBER)
		sValue = luaValue.toValue<string>();
	else
		sValue = luaValue.typeName();

	SetError("can not convert field " + PopFullName() +
		"(" + sValue + ") to " + (bToInt ? "integer" : "float"));
}
