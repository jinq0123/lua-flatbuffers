#include "name_stack.h"

std::string NameStack::PopFullFieldName(const std::string& sFieldName)
{
	std::string sFullName = PopFullName();
	if (sFullName.empty()) return sFieldName;
	return sFullName + "." + sFieldName;
}

std::string NameStack::PopFullName()
{
	if (empty()) return "";

	std::string sResult(top());
	pop();
	while (!empty())
	{
		sResult.insert(0, top() + ".");
		pop();
	}
	return sResult;
}

void NameStack::Reset()
{
	swap(NameStack());  // swap to clear
}
