#include "name_stack.h"

std::string NameStack::PopFullFieldName(const std::string& sFieldName)
{
	std::string sFullName = PopFullName();
	if (sFullName.empty()) return sFieldName;
	return sFullName + "." + sFieldName;
}

std::string NameStack::PopFullName()
{
	if (m_stack.empty()) return "";

	std::string sResult(m_stack.top());
	m_stack.pop();
	while (!m_stack.empty())
	{
		sResult.insert(0, m_stack.top() + ".");
		m_stack.pop();
	}
	return sResult;
}

void NameStack::Reset()
{
	m_stack.swap(Stack());  // swap to clear
}
