#ifndef LUA_FLATBUFFERS_DECODER_BASE_H_
#define LUA_FLATBUFFERS_DECODER_BASE_H_

#include <flatbuffers/flatbuffers.h>

#define ERR_RET_NIL(ErrorStr) do { \
	SetError(ErrorStr); \
	return Nil(); \
} while(0)

class NameStack;

namespace LuaIntf {
class LuaRef;
}

namespace reflection {
struct Object;
struct Schema;
}

struct lua_State;

class DecoderBase
{
public:
	DecoderBase(lua_State* state,
		const reflection::Schema& schema,
		flatbuffers::Verifier& rVerifier,
		NameStack& rNameStack);

public:
	using LuaRef = LuaIntf::LuaRef;
	using string = std::string;

protected:
	bool Bad() const { return !m_sError.empty(); }
	LuaRef Nil() const;
	void SetError(const string& sError);
	string PopFullName();
	string PopFullFieldName(const string& sFieldName);
	string PopFullVectorName(size_t index);

protected:
	lua_State* m_pLuaState;
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<
		reflection::Object>>& m_vObjects;

	flatbuffers::Verifier& m_rVerifier;
	NameStack& m_rNameStack;  // For error message.

	string m_sError;
};  // class Decoder

#endif  // LUA_FLATBUFFERS_DECODER_BASE_H_
