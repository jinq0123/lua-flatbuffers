#include "union_decoder.h"

#include "object_decoder.h"  // for ObjectDecoder
#include "vector_decoder.h"  // for VectorDecoder

LuaIntf::LuaRef UnionDecoder::DecodeUnion(
	const reflection::Type& type, const void* pData)
{
	assert(pData);
	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		return DecodeScalar<uint8_t>(pData);
	case reflection::Byte:
		return DecodeScalar<int8_t>(pData);
	case reflection::Short:
		return DecodeScalar<int16_t>(pData);
	case reflection::UShort:
		return DecodeScalar<uint16_t>(pData);
	case reflection::Int:
		return DecodeScalar<uint8_t>(pData);
	case reflection::UInt:
		return DecodeScalar<uint8_t>(pData);
	case reflection::Long:
		return DecodeScalar<int64_t>(pData);
	case reflection::ULong:
		return DecodeScalar<uint64_t>(pData);
	case reflection::Float:
		return DecodeScalar<float>(pData);
	case reflection::Double:
		return DecodeScalar<double>(pData);

	case reflection::String:
	{
		const auto* pStr = reinterpret_cast<const flatbuffers::String*>(pData);
		if (!Verifier().Verify(pStr))
			ERR_RET_NIL("illegal string " + PopFullName());
		return LuaRef::fromValue(LuaState(), pStr->str());
	}
	case reflection::Vector:
	{
		const auto* pVec = reinterpret_cast<
			const flatbuffers::VectorOfAny*>(pData);
		return VectorDecoder(m_rCtx).DecodeVector(type, *pVec);
	}
	case reflection::Obj:
		return ObjectDecoder(m_rCtx).DecodeObject(*Objects()[type.index()], pData);
	case reflection::Union:
		assert(!"Union must always be part of a table.");
	}  // switch

	assert(false);
	return Nil();
}

template <typename T>
inline LuaIntf::LuaRef UnionDecoder::DecodeScalar(const void* pData)
{
	assert(pData);
	if (!Verifier().Verify<T>(pData))
		ERR_RET_NIL("illegal scalar " + PopFullName());
	return LuaRef::fromValue(LuaState(), flatbuffers::ReadScalar<T>(pData));
}
