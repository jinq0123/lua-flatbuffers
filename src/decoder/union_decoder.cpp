#include "union_decoder.h"

#include "object_decoder.h"  // for ObjectDecoder
#include "vector_decoder.h"  // for VectorDecoder

LuaIntf::LuaRef UnionDecoder::DecodeUnion(const reflection::Type& type,
	int64_t nUnionType, const void* pData)
{
	assert(reflection::Union == type.base_type());

	const reflection::Enum* pEnum = (*m_rCtx.schema.enums())[type.index()];
	assert(pEnum);
	assert(pEnum->is_union());
	const reflection::EnumVal* pEnumVal =
		pEnum->values()->LookupByKey(nUnionType);
	if (!pEnumVal) return Nil();  // Allow schema version compatibility.
	const reflection::Object* pObj = pEnumVal->object();
	assert(pObj);
	// only tables can be union elements
	assert(!pObj->is_struct());
	return ObjectDecoder(m_rCtx).DecodeObject(*pObj, pData);
}

