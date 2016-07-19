#ifndef LUA_FLATBUFFERS_SCHEMA_CACHE_H_
#define LUA_FLATBUFFERS_SCHEMA_CACHE_H_

#include <memory>  // for shared_ptr<>
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
	using Schema = reflection::Schema;

	// Find the schema that contains the object.
	const reflection::Schema* GetSchemaOfObject(const string& sObjName) const;

private:
	// Binary FlatBuffers schema shared_ptr.
	using BfbsSptr = std::shared_ptr<string>;
	using Str2Bfbs = std::unordered_map<string, BfbsSptr>;
	Str2Bfbs m_mapFile2Bfbs;  // FileName -> BfbsSptr
	Str2Bfbs m_mapObjName2Bfbs;  // ObjectName -> BfbsSptr
};

#endif  // LUA_FLATBUFFERS_SCHEMA_CACHE_H_
