#include "decoder_base.h"

#include "name_stack.h"

#include <flatbuffers/reflection.h>  // for Schema

#include <sstream>

using LuaIntf::LuaRef;

DecoderBase::DecoderBase(DecoderContext& rCtx) : m_rCtx(rCtx)
{
	assert(rCtx.pLuaState);
}

DecoderBase::~DecoderBase()
{
}

LuaRef DecoderBase::Nil() const
{
	return LuaRef(m_rCtx.pLuaState, nullptr);
}

void DecoderBase::SetError(const std::string& sError)
{
	m_rCtx.sError = sError;
}

std::string DecoderBase::PopFullName()
{
	return m_rCtx.nameStack.PopFullName();
}

std::string DecoderBase::PopFullFieldName(const std::string& sFieldName)
{
	return m_rCtx.nameStack.PopFullFieldName(sFieldName);
}

std::string DecoderBase::PopFullVectorName(size_t index)
{
	std::ostringstream oss;
	oss << "[" << index << "]";
	return PopFullName() + oss.str();
}

