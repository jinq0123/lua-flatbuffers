#ifndef LUA_FLATBUFFERS_ENCODER_ENCODER_CONTEXT_H_
#define LUA_FLATBUFFERS_ENCODER_ENCODER_CONTEXT_H_

#include "name_stack.h"  // for NameStack

#include <flatbuffers/flatbuffers.h>  // for FlatBufferBuilder

#include <string>

namespace reflection {
struct Schema;
}

struct lua_State;

struct EncoderContext
{
	const reflection::Schema& schema;
	flatbuffers::FlatBufferBuilder fbb;
	NameStack nameStack;  // For error message.
	std::string sError;
};  // class EncoderContext

#endif  // LUA_FLATBUFFERS_ENCODER_ENCODER_CONTEXT_H_
