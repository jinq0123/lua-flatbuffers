#ifndef LUA_FLATBUFFERS_DECODER_CONTEXT_H_
#define LUA_FLATBUFFERS_DECODER_CONTEXT_H_

#include "name_stack.h"

#include <flatbuffers/flatbuffers.h>

#include <string>

namespace reflection {
struct Schema;
}

struct lua_State;

struct DecoderContext
{
	lua_State* pLuaState;
	const reflection::Schema& schema;
	flatbuffers::Verifier verifier;
	NameStack nameStack;  // For error message.
	std::string sError;
};  // class DecoderContext

#endif  // LUA_FLATBUFFERS_DECODER_CONTEXT_H_
