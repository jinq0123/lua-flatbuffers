#include "union_decoder.h"

#include "object_decoder.h"  // for ObjectDecoder
#include "vector_decoder.h"  // for VectorDecoder

LuaIntf::LuaRef UnionDecoder::DecodeUnion(
	const reflection::Type& type, const void* pData)
{
	assert(pData);
	using namespace reflection;
	switch (type.base_type())
	{
	case UType:
	case Bool:
	case UByte:
		return DecodeScalar<uint8_t>(pData);
	case Byte:
		return DecodeScalar<int8_t>(pData);
	case Short:
		return DecodeScalar<int16_t>(pData);
	case UShort:
		return DecodeScalar<uint16_t>(pData);
	case Int:
		return DecodeScalar<uint8_t>(pData);
	case UInt:
		return DecodeScalar<uint8_t>(pData);
	case Long:
		return DecodeScalar<int64_t>(pData);
	case ULong:
		return DecodeScalar<uint64_t>(pData);
	case Float:
		return DecodeScalar<float>(pData);
	case Double:
		return DecodeScalar<double>(pData);

	case String:
	{
		const auto* pStr = reinterpret_cast<const flatbuffers::String*>(pData);
		if (!Verifier().Verify(pStr))
			ERR_RET_NIL("illegal string " + PopFullName());
		return LuaRef::fromValue(LuaState(), pStr->str());
	}
	case Vector:
	{
		const auto* pVec = reinterpret_cast<
			const flatbuffers::VectorOfAny*>(pData);
		return VectorDecoder(m_rCtx).DecodeVector(type, *pVec);
	}
	case Obj:
		return ObjectDecoder(m_rCtx).DecodeObject(*Objects()[type.index()], pData);
	case Union:
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
