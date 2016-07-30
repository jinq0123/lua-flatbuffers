#ifndef LUA_FLATBUFFERS_ENCODER_XXX_H_
#define LUA_FLATBUFFERS_ENCODER_XXX_H_

#include "encoder_base.h"  // EncoderBase

#include <unordered_map>

class XXXEncoder final : EncoderBase
{
public:
	explicit XXXEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

private:
	// Encode recursively. Return 0 and set m_sError if any error.
	using Object = reflection::Object;
	using uoffset_t = flatbuffers::uoffset_t;
	using Field = reflection::Field;

};  // class XXXEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_XXX_H_
