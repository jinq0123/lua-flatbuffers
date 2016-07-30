#include "encoder.h"

#include "object_encoder.h"  // for ObjectEncoder

bool Encoder::Encode(const string& sName, const LuaRef& luaTable)
{
	if (!luaTable.isTable())
		ERR_RET_FALSE("lua data is not table");

	const reflection::Object* pObj = Objects().LookupByKey(sName.c_str());
	assert(pObj);
	PushName(sName);
	flatbuffers::uoffset_t offset =
		ObjectEncoder(m_rCtx).EncodeObject(*pObj, luaTable);
	SafePopName();
	if (!offset)  // An offset of 0 means error.
		return false;

	// Todo: Add file_identifier if root_type
	Builder().Finish(flatbuffers::Offset<void>(offset));
	return true;
}

