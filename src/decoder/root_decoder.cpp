#include "root_decoder.h"

#include "object_decoder.h"  // for ObjectDecoder

LuaIntf::LuaRef RootDecoder::Decode(const string& sName, const void* pBuf)
{
	if (!pBuf) ERR_RET_NIL("nil buffer");

	// Check the first offset field before GetRoot().
	if (!Verifier().Verify<flatbuffers::uoffset_t>(pBuf))
		ERR_RET_NIL("buffer is too short");

	const void* pRoot = flatbuffers::GetRoot<void>(pBuf);
	if (!pRoot)
		ERR_RET_NIL("illegal root");

	const reflection::Object* pObj = Objects().LookupByKey(sName.c_str());
	assert(pObj);
	
	return ObjectDecoder(m_rCtx).DecodeObject(*pObj, pRoot);
}
