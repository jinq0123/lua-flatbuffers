#ifndef LUA_FLATBUFFERS_SCHEMA_CACHE_H_
#define LUA_FLATBUFFERS_SCHEMA_CACHE_H_

#include <string>
#include <tuple>

// Cache FlatBuffers schemas.
class SchemaCache
{
public:
	SchemaCache();
	virtual ~SchemaCache();

public:
	using string = std::string;
	std::tuple<bool, string> LoadBfbsFile(const string& sBfbsFile);
	std::tuple<bool, string> LoadBfbs(const string& sBfbs);
	std::tuple<bool, string> LoadFbsFile(const string& sFbsFile);
	std::tuple<bool, string> LoadFbs(const string& sFbs);

private:
};

#endif  // LUA_FLATBUFFERS_SCHEMA_CACHE_H_
