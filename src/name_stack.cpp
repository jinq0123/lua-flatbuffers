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

	std::string sResult;
	while (!m_stack.empty())
	{
		// No '.' for vector index. Like: VecFld[n], not VecFld.[n]
		if (!sResult.empty() && '[' != sResult[0])
			sResult.insert(0, ".");
		sResult.insert(0, m_stack.top());
		m_stack.pop();
	}
	return sResult;
}

void NameStack::Reset()
{
	m_stack.swap(Stack());  // swap to clear
}
