#include "union_decoder.h"

#include "object_decoder.h"  // for ObjectDecoder
#include "vector_decoder.h"  // for VectorDecoder

#include <sstream>  // for ostringstream

LuaIntf::LuaRef UnionDecoder::DecodeUnion(const reflection::Type& type,
	int64_t nUnionType, const void* pData)
{
	assert(reflection::Union == type.base_type());

	const reflection::Enum* pEnum = (*m_rCtx.schema.enums())[type.index()];
	assert(pEnum);
	assert(pEnum->is_union());
	assert(pEnum->values()->LookupByKey(0));  // Union type: 1..n, 0(Unknown)?
	assert(pEnum->values()->LookupByKey(0)->object() == nullptr);

	const reflection::EnumVal* pEnumVal =
		pEnum->values()->LookupByKey(nUnionType);
	if (!pEnumVal) return Nil();  // Allow schema version compatibility.
	const reflection::Object* pObj = pEnumVal->object();
	if (pObj)
	{
		// only tables can be union elements
		assert(!pObj->is_struct());
		return ObjectDecoder(m_rCtx).DecodeObject(*pObj, pData);
	}

	std::ostringstream oss;
	oss << "illegal union type " << PopFullName()
		<< flatbuffers::UnionTypeFieldSuffix() << "(" << nUnionType << ")";
	ERR_RET_NIL(oss.str());
}
