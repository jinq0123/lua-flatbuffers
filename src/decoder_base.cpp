#include "decoder_base.h"

#include "name_stack.h"

#include <flatbuffers/reflection.h>  // for Schema
#include <LuaIntf/LuaIntf.h>

#include <sstream>

using LuaIntf::LuaRef;

DecoderBase::DecoderBase(lua_State* state,
	const reflection::Schema& schema,
	flatbuffers::Verifier& rVerifier,
	NameStack& rNameStack) :
	m_pLuaState(state),
	m_schema(schema),
	m_vObjects(*schema.objects()),
	m_rVerifier(rVerifier),
	m_rNameStack(rNameStack)
{
	assert(state);
}

LuaRef DecoderBase::Nil() const
{
	return LuaRef(m_pLuaState, nullptr);
}

void DecoderBase::SetError(const std::string& sError)
{
	m_sError = sError;
}

std::string DecoderBase::PopFullName()
{
	return m_rNameStack.PopFullName();
}

std::string DecoderBase::PopFullFieldName(const std::string& sFieldName)
{
	return m_rNameStack.PopFullFieldName(sFieldName);
}

std::string DecoderBase::PopFullVectorName(size_t index)
{
	std::ostringstream oss;
	oss << "[" << index << "]";
	return PopFullName() + oss.str();
}

