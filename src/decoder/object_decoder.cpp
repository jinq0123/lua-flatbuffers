#include "object_decoder.h"

#include "struct_decoder.h"
#include "table_decoder.h"

ObjectDecoder::ObjectDecoder(DecoderContext& rCtx) : DecoderBase(rCtx)
{
}

LuaIntf::LuaRef ObjectDecoder::DecodeObject(
	const reflection::Object& object,
	const void* pData)
{
	if (!pData) return Nil();
	if (object.is_struct())
	{
		return StructDecoder(m_rCtx).DecodeStruct(object,
			*reinterpret_cast<const flatbuffers::Struct*>(pData));
	}
	return TableDecoder(m_rCtx).DecodeTable(object,
		*reinterpret_cast<const Table*>(pData));
}
