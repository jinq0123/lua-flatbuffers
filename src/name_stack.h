#ifndef LUA_FLATBUFFERS_NAME_STACK_H_
#define LUA_FLATBUFFERS_NAME_STACK_H_

#include <stack>
#include <string>

// Objects name stack for error message.
class NameStack final
{
public:
	void Push(const std::string& s) { m_stack.push(s); }
	void SafePop()
	{
		if (!m_stack.empty()) m_stack.pop();
	}

	std::string PopFullFieldName(const std::string& sFieldName);
	std::string PopFullName();
	void Reset();

private:
	using Stack = std::stack<std::string>;
	Stack m_stack;
};

#endif  // LUA_FLATBUFFERS_NAME_STACK_H_
