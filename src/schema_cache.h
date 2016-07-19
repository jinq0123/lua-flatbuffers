#ifndef LUA_FLATBUFFERS_SCHEMA_CACHE_H_
#define LUA_FLATBUFFERS_SCHEMA_CACHE_H_

#include <string>
#include <tuple>
#include <unordered_map>

namespace reflection {
struct Schema;
}

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

public:
	// Find the schema that contains the object.
	const reflection::Schema *GetSchemaOfObject(const std::string& sObjName);

private:
	using File2Bfbs = std::unordered_map<std::string, std::string>;
	File2Bfbs m_mapFile2Bfbs;
};

#endif  // LUA_FLATBUFFERS_SCHEMA_CACHE_H_
