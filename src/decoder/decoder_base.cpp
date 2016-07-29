#include "decoder_base.h"

#include "name_stack.h"

#include <sstream>  // for ostringstream

LuaIntf::LuaRef DecoderBase::Nil() const
{
	return LuaRef(m_rCtx.pLuaState, nullptr);
}

void DecoderBase::SetError(const string& sError)
{
	m_rCtx.sError = sError;
}

std::string DecoderBase::PopFullName()
{
	return m_rCtx.nameStack.PopFullName();
}

std::string DecoderBase::PopFullFieldName(const string& sFieldName)
{
	return m_rCtx.nameStack.PopFullFieldName(sFieldName);
}

std::string DecoderBase::PopFullVectorName(size_t index)
{
	std::ostringstream oss;
	oss << "[" << index << "]";
	return PopFullName() + oss.str();
}

