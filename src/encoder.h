#ifndef LUA_FLATBUFFERS_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_H_

#include <flatbuffers/flatbuffers.h>

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
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>& m_vObjects;
};  // class Encoder

#endif  // LUA_FLATBUFFERS_ENCODER_H_
