#ifndef LUA_FLATBUFFERS_ENCODER_XXX_H_
#define LUA_FLATBUFFERS_ENCODER_XXX_H_

#include "encoder_base.h"  // EncoderBase

class XXXEncoder final : EncoderBase
{
public:
	explicit XXXEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

private:
	// Encode recursively. Return 0 and set m_sError if any error.
	using Object = reflection::Object;
	using uoffset_t = flatbuffers::uoffset_t;
	using Field = reflection::Field;
	uoffset_t EncodeObject(const Object& obj, const LuaRef& luaTable);

	uoffset_t EncodeStruct(const Object& obj, const LuaRef& luaTable);
	bool CheckStructFields(const Object& obj, const LuaRef& luaTable);
	bool EncodeStructToBuf(const Object& obj,
		const LuaRef& luaTable, uint8_t* pBuf);
	bool EncodeStructFieldToBuf(const Field& field,
		const LuaRef& luaTable, uint8_t* pBuf);
	void EncodeStructElementToBuf(reflection::BaseType eType,
		const LuaRef& luaValue, uint8_t* pDest);

	uoffset_t EncodeTable(const Object& obj, const LuaRef& luaTable);
	uoffset_t EncodeVector(const reflection::Type& type, const LuaRef& luaArray);

	// Cache to map before StartTable().
	// Field2Lua caches scalar and struct LuaRef.
	using Field2Lua = std::unordered_map<const Field*, LuaRef>;
	using Field2Offset = std::unordered_map<const Field*, uoffset_t>;
	bool CacheFields(const Object& obj, const LuaRef& luaTable,
		Field2Lua& rMapLuaRef, Field2Offset& rMapOffset);
	void CacheField(const Field* pField, const LuaRef& luaValue,
		Field2Lua& rMapLuaRef, Field2Offset& rMapOffset);

	void AddElements(const Field2Lua& mapScalar);
	void AddOffsets(const Field2Offset& mapOffset);
	void AddElement(const Field& field, const LuaRef& value);

	template <typename ElementType, typename DefaultValueType>
	inline void AddElement(uint16_t offset, const LuaRef& elementValue,
		DefaultValueType defaultValue);

private:
	bool CheckObjectField(const Field* pField, const string& sFieldName);
};  // class XXXEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_XXX_H_
