#ifndef LUA_FLATBUFFERS_DECODER_DECODER_BASE_H_
#define LUA_FLATBUFFERS_DECODER_DECODER_BASE_H_

#include "decoder_context.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/reflection.h>  // for objects()
#include <LuaIntf/LuaIntf.h>  // for createTable()

#include <string>

#define ERR_RET_NIL(ErrorStr) do { \
	SetError(ErrorStr); \
	return Nil(); \
} while(0)

namespace LuaIntf {
class LuaRef;
}

class DecoderBase
{
public:
	explicit DecoderBase(DecoderContext& rCtx);

public:
	using LuaRef = LuaIntf::LuaRef;
	using string = std::string;

protected:
	flatbuffers::Verifier& Verifier() { return m_rCtx.verifier; }

	const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>&
	Objects() const
	{
		return *m_rCtx.schema.objects();
	}
	lua_State* LuaState() const { return m_rCtx.pLuaState; }
	LuaRef CreateLuaTable() const { return LuaRef::createTable(LuaState()); }

protected:
	bool Bad() const { return !m_rCtx.sError.empty(); }
	LuaRef Nil() const;
	void SetError(const string& sError);
	void PushName(const std::string& sName) { m_rCtx.nameStack.Push(sName); }
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
	string PopFullVectorName(size_t index);

protected:
	DecoderContext& m_rCtx;
};  // class DecoderBase

#endif  // LUA_FLATBUFFERS_DECODER_DECODER_BASE_H_
