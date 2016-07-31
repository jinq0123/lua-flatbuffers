#include "encoder.h"

#include "struct_encoder.h"
#include "table_encoder.h"

bool Encoder::Encode(const string& sName, const LuaRef& luaTable)
{
	if (!luaTable.isTable())
	{
		SetError(string("lua data is not table but ") + luaTable.typeName());
		return false;
	}

	const reflection::Object* pObj = Objects().LookupByKey(sName.c_str());
	if (!pObj)
	{
		SetError("schema has no type " + sName);  // wrong schema
		return false;
	}
	PushName(sName);
	flatbuffers::uoffset_t offset =
		pObj->is_struct() ?
		StructEncoder(m_rCtx).EncodeStruct(*pObj, luaTable) :
		TableEncoder(m_rCtx).EncodeTable(*pObj, luaTable);
	SafePopName();
	if (Bad()) return false;

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
