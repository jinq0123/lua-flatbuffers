#include "schema_cache.h"

SchemaCache::SchemaCache() {}
SchemaCache::~SchemaCache() {}

std::tuple<bool, std::string>
SchemaCache::LoadBfbsFile(const std::string& sBfbsFile)
{
	return std::make_tuple(false, "To be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadBfbs(const std::string& sBfbs)
{
	return std::make_tuple(false, "To be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadFbsFile(const std::string& sFbsFile)
{
	return std::make_tuple(false, "To be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadFbs(const std::string& sFbs)
{
	return std::make_tuple(false, "To be implemented");
}

