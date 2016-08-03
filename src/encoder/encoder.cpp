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

#ifndef NDEBUG
class Test : public EncoderBase
{
public:
	Test(EncoderContext& rCtx) : EncoderBase(rCtx)
	{
	}

	template <typename T>
	LuaRef ToNum(const LuaRef& luaVal)
	{
		m_rCtx.nameStack.Reset();
		PushName("test");
		m_rCtx.sError.clear();
		T num = LuaToNumber<T>(luaVal);

		lua_State* L = luaVal.state();
		if (Bad()) return LuaRef::fromValue(L, m_rCtx.sError);
		return LuaRef::fromValue(L, num);
	}
};
#endif

LuaIntf::LuaRef Encoder::TestToNum(const LuaRef& luaVal)
{
	lua_State* L = luaVal.state();

#ifdef NDEBUG
	return LufRef(L, nullptr);
#else
	char buf[1];
	auto* pSchema = reinterpret_cast<reflection::Schema*>(buf);
	EncoderContext ctx{*pSchema};

	LuaRef tb = LuaRef::createTable(L);
	Test test(ctx);
	tb["int8"] = test.ToNum<int8_t>(luaVal);
	tb["uint8"] = test.ToNum<uint8_t>(luaVal);
	tb["int16"] = test.ToNum<int16_t>(luaVal);
	tb["uint16"] = test.ToNum<uint16_t>(luaVal);
	tb["int32"] = test.ToNum<int32_t>(luaVal);
	tb["uint32"] = test.ToNum<uint32_t>(luaVal);
	tb["int64"] = test.ToNum<int64_t>(luaVal);
	tb["uint64"] = test.ToNum<uint64_t>(luaVal);
	tb["float"] = test.ToNum<float>(luaVal);
	tb["double"] = test.ToNum<double>(luaVal);
	return tb;
#endif
}

