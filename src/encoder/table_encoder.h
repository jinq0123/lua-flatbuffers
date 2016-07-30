#ifndef LUA_FLATBUFFERS_ENCODER_TABLE_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_TABLE_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

#include <unordered_map>

class TableEncoder final : EncoderBase
{
public:
	explicit TableEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	using Object = reflection::Object;
	using uoffset_t = flatbuffers::uoffset_t;

	// Encode recursively. Return 0 and set sError if any error.
	uoffset_t EncodeTable(const Object& obj, const LuaRef& luaTable);

private:
	using Field = reflection::Field;

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
};  // class TableEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_TABLE_ENCODER_H_
