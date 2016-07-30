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

