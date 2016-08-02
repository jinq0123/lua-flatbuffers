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

template <>
float EncoderBase::LuaToNumber<float>(const LuaRef& luaValue)
{
	CheckScalarLuaValue(luaValue);
	if (Bad()) return 0.0;
	double dVal = luaValue.toValue<double>();
	if (dVal >= std::numeric_limits<float>::min() &&
		dVal <= std::numeric_limits<float>::max())
		return static_cast<float>(dVal);

	std::ostringstream oss;
	oss << "float field " << PopFullName() << "("
		<< luaValue.toValue<string>() << ") is out of range ["
		<< std::numeric_limits<float>::min() << ", "
		<< std::numeric_limits<float>::max() << ")";
	SetError(oss.str());
	return 0.0;
}

template <>
double EncoderBase::LuaToNumber<double>(const LuaRef& luaValue)
{
	CheckScalarLuaValue(luaValue);
	if (Bad()) return 0.0;
	return luaValue.toValue<double>();  // allow int64 -> double
}

