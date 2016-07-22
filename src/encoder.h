#ifndef LUA_FLATBUFFERS_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_H_

#include <flatbuffers/flatbuffers.h>

#include <stack>
#include <unordered_map>

namespace LuaIntf {
class LuaRef;
}

namespace reflection {
struct Field;
struct Object;
struct Schema;
}

class Encoder final
{
public:
	explicit Encoder(const reflection::Schema& schema);

public:
	using LuaRef = LuaIntf::LuaRef;
	std::tuple<bool, std::string> Encode(
		const std::string& sName, const LuaRef& table);

private:
	// Encode recursively. Return 0 and set m_sError if any error.
	using Object = reflection::Object;
	flatbuffers::uoffset_t Encoder::Encode(
		const Object& obj, const LuaRef& table);

	// Cache to map before StartTable().
	using Field2Scalar = std::unordered_map<
		const reflection::Field*, LuaRef>;
	using Field2Offset = std::unordered_map<
		const reflection::Field*, flatbuffers::uoffset_t>;
	bool CacheFields(const Object& obj, const LuaRef& table,
		Field2Scalar& rMapScalar, Field2Offset& rMapOffset);
	void AddElements(const Field2Scalar& mapScalar);
	void AddOffsets(const Field2Offset& mapOffset);
	void AddElement(const reflection::Field& field, const LuaRef& value);

	template <typename ElementType, typename DefaultValueType>
	inline void AddElement(uint16_t offset, const LuaRef& elementValue,
		DefaultValueType defaultValue);

private:
	std::string GetFullFieldName(const std::string& sFieldName) const;
	void Reset();

private:
	flatbuffers::FlatBufferBuilder m_fbb;
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<Object>>& m_vObjects;
	using NameStack = std::stack<std::string>;
	NameStack m_nameStack;  // For error message.
	std::string m_sError;  // Encode error.

};  // class Encoder

#endif  // LUA_FLATBUFFERS_ENCODER_H_
