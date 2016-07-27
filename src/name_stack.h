#ifndef LUA_FLATBUFFERS_NAME_STACK_H_
#define LUA_FLATBUFFERS_NAME_STACK_H_

#include <stack>
#include <string>

// Objects name stack for error message.
class NameStack final : public std::stack<std::string>
{
public:
	std::string PopFullFieldName(const std::string& sFieldName);
	std::string PopFullName();
};

#endif  // LUA_FLATBUFFERS_NAME_STACK_H_
