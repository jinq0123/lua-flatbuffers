#include "encoder.h"

#include "struct_encoder.h"
#include "table_encoder.h"

bool Encoder::Encode(const string& sName, const LuaRef& luaTable)
{
	if (!luaTable.isTable())
		ERR_RET_FALSE("lua data is not table");

	const reflection::Object* pObj = Objects().LookupByKey(sName.c_str());
	assert(pObj);
	PushName(sName);
	flatbuffers::uoffset_t offset =
		pObj->is_struct() ?
		StructEncoder(m_rCtx).EncodeStruct(*pObj, luaTable) :
		TableEncoder(m_rCtx).EncodeTable(*pObj, luaTable);
	SafePopName();
	if (!offset)  // An offset of 0 means error.
		return false;

	// Todo: Add file_identifier if root_type
	Builder().Finish(flatbuffers::Offset<void>(offset));
	return true;
}

std::string Encoder::GetResultStr() const
{
	const char* pBuffer = reinterpret_cast<const char*>(
		Builder().GetBufferPointer());
	return string(pBuffer, Builder().GetSize());
}
