#ifndef LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_
#define LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_

#include "encoder_context.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/reflection.h>  // for objects()
#include <LuaIntf/LuaIntf.h>  // for createTable()

#include <string>

#define ERR_RET_FALSE(ErrorStr) do { \
	SetError(ErrorStr); \
	return false; \
} while(0)

class EncoderBase
{
public:
	explicit EncoderBase(EncoderContext& rCtx) : m_rCtx(rCtx) {};
	virtual ~EncoderBase() {};

public:
	using LuaRef = LuaIntf::LuaRef;
	using string = std::string;

protected:
	const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>&
	Objects() const
	{
		return *m_rCtx.schema.objects();
	}
	flatbuffers::FlatBufferBuilder& Builder() { return m_rCtx.fbb; }
	const flatbuffers::FlatBufferBuilder& Builder() const { return m_rCtx.fbb; }

protected:
	bool Bad() const { return !m_rCtx.sError.empty(); }
	void SetError(const string& sError);
	void PushName(const string& sName) { m_rCtx.nameStack.Push(sName); }
	void PushName(const reflection::Object& object)
	{
		PushName(object.name()->c_str());
	}
	void PushName(const reflection::Field& field)
	{
		PushName(field.name()->c_str());
	}
	void SafePopName() { m_rCtx.nameStack.SafePop(); }
	string PopFullName();
	string PopFullFieldName(const string& sFieldName);
	string PopFullFieldName(const reflection::Field& field);
	string PopFullVectorName(size_t index);

protected:
	EncoderContext& m_rCtx;
};  // class EncoderBase

#endif  // LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_
