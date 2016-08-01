#include "vector_encoder.h"

flatbuffers::uoffset_t VectorEncoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	assert(type.base_type() == reflection::Vector);
	// XXX: check luaArray is array
	return 0;
}


