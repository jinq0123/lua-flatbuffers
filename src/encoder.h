#ifndef LUA_FLATBUFFERS_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_H_

#include <flatbuffers/flatbuffers.h>
#include <stack>

namespace LuaIntf {
class LuaRef;
}

namespace reflection {
struct Object;
struct Schema;
}

class Encoder final
{
public:
	explicit Encoder(const reflection::Schema& schema);

public:
	std::tuple<bool, std::string> Encode(
		const std::string& sName, const LuaIntf::LuaRef& table);

private:
	std::tuple<bool, std::string> Build(
		const std::string& sName, const LuaIntf::LuaRef& table);

private:
	std::string GetFullFieldName(const std::string& sFieldName) const;

private:
	flatbuffers::FlatBufferBuilder m_fbb;
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>& m_vObjects;
	using NameStack = std::stack<std::string>;
	NameStack m_nameStack;  // For error message.
};  // class Encoder

#endif  // LUA_FLATBUFFERS_ENCODER_H_
