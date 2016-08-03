#include "encoder_base.h"

#include "name_stack.h"  // for PopFullName()

#include <sstream>  // for ostringstream

void EncoderBase::SetError(const string& sError)
{
	m_rCtx.sError = sError;
}

std::string EncoderBase::PopFullName()
{
	return m_rCtx.nameStack.PopFullName();
}

std::string EncoderBase::PopFullFieldName(const string& sFieldName)
{
	return m_rCtx.nameStack.PopFullFieldName(sFieldName);
}

std::string EncoderBase::PopFullFieldName(const reflection::Field& field)
{
	return PopFullFieldName(field.name()->c_str());
}

std::string EncoderBase::PopFullVectorName(size_t index)
{
	std::ostringstream oss;
	oss << "[" << index << "]";
	return PopFullName() + oss.str();
}

template <> int64_t EncoderBase::LuaToNumber<int64_t>(const LuaRef& luaValue)
{
	CheckScalarLuaValue(luaValue);
	if (Bad()) return 0;
	double dVal = luaValue.toValue<double>();
	if (IsInteger(dVal))
		return luaValue.toValue<int64_t>();
	return static_cast<int64_t>(dVal);
}

template <> uint64_t EncoderBase::LuaToNumber<uint64_t>(const LuaRef& luaValue)
{
	int64_t l = LuaToNumber<int64_t>(luaValue);
	if (Bad()) return 0;
	return static_cast<uint64_t>(l);
}

template <> double EncoderBase::LuaToNumber<double>(const LuaRef& luaValue)
{
	CheckScalarLuaValue(luaValue);
	if (Bad()) return 0.0;
	return luaValue.toValue<double>();  // allow int64 -> double
}

bool EncoderBase::IsInteger(double dValue) const
{
	double dFract, dInt;
	dFract = modf(dValue, &dInt);
	return 0.0 == dFract;
}
