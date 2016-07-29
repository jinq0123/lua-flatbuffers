#include "object_decoder.h"

#include "struct_decoder.h"  // for StructDecoder
#include "table_decoder.h"  // for TableDecoder

LuaIntf::LuaRef ObjectDecoder::DecodeObject(
	const reflection::Object& object, const void* pData)
{
	if (!pData) return Nil();
	if (object.is_struct())
	{
		return StructDecoder(m_rCtx).DecodeStruct(object,
			*reinterpret_cast<const flatbuffers::Struct*>(pData));
	}
	return TableDecoder(m_rCtx).DecodeTable(object,
		*reinterpret_cast<const flatbuffers::Table*>(pData));
}
