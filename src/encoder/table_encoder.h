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

	void CacheFields(const Object& obj);
	void CacheField(const Field* pField, const LuaRef& luaValue);
	void CacheObjField(const Field* pField, const LuaRef& luaValue);
	void CacheUnionField(const Field* pField, const LuaRef& luaValue);

	void EncodeCachedStructs();
	void EncodeStruct(const Field& field, const LuaRef& luaValue);
	void EncodeCachedScalars();
	void EncodeScalar(const Field& field, const LuaRef& luaValue);
	void EncodeCachedOffsets();

	template <typename ElementType, typename DefaultValueType>
	inline void AddElement(uint16_t offset, const LuaRef& elementValue,
		DefaultValueType defaultValue);

private:
	// Cache to map before StartTable().
	// Field2Lua caches scalar and struct LuaRef.
	using Field2Lua = std::unordered_map<const Field*, LuaRef>;
	using Field2Offset = std::unordered_map<const Field*, uoffset_t>;
	Field2Lua m_mapScalars;
	Field2Lua m_mapStructs;
	Field2Offset m_mapOffsets;

	const LuaRef* m_pLuaTable;
};  // class TableEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_TABLE_ENCODER_H_
