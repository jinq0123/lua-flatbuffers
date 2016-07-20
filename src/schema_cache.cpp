#include "schema_cache.h"

#include <flatbuffers/reflection_generated.h>  // for Schema
#include <flatbuffers/util.h>  // for LoadFile()

SchemaCache::SchemaCache() {}
SchemaCache::~SchemaCache() {}

static bool LoadFile(const std::string sPath, std::string& rContents) {
	std::ifstream ifs(sPath.c_str(), std::ifstream::binary);
	if (!ifs.is_open()) return false;
	ifs.seekg(0, std::ios::end);
	size_t size = static_cast<size_t>(ifs.tellg());
	rContents.resize(size);
	ifs.seekg(0, std::ios::beg);
	ifs.read(&rContents[0], size);
	return !ifs.bad();
}

std::tuple<bool, std::string>
SchemaCache::LoadBfbsFile(const string& sBfbsFile)
{
	std::string sContents;
	if (LoadFile(sBfbsFile, sContents))
		return LoadBfbs(sContents);
	return std::make_tuple(false, "unable to load file");
}

std::tuple<bool, std::string>
SchemaCache::LoadBfbs(const string& sBfbs)
{
	if (m_setLoadedBfbs.find(sBfbs) != m_setLoadedBfbs.end())
		return std::make_tuple(true, "");

	// Verify it.
	flatbuffers::Verifier verifier(
		reinterpret_cast<const uint8_t *>(sBfbs.data()), sBfbs.length());
	if (!reflection::VerifySchemaBuffer(verifier))
		return std::make_tuple(false, "failed to verify schema buffer");

	BfbsSptr pBfbs(new string(sBfbs));
	const Schema* pSchema = flatbuffers::GetRoot<Schema>(pBfbs->data());
	assert(pSchema);
	const auto& vObjects = *pSchema->objects();
	for (const auto* pObj : vObjects)
	{
		assert(pObj);
		const string& sName = pObj->name()->str();
		//if (m_mapObjName2Bfbs[sName])
		//{
		//	string sError = "2 objects name '" + sName + "'";
		//	return std::make_tuple(false, sError);
		//}
		m_mapObjName2Bfbs[sName] = pBfbs;
	}

	m_setLoadedBfbs.insert(sBfbs);
	return std::make_tuple(true, "");
}

std::tuple<bool, std::string>
SchemaCache::LoadFbsFile(const string& sFbsFile)
{
	return std::make_tuple(false, "to be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadFbs(const string& sFbs)
{
	return std::make_tuple(false, "to be implemented");
}

const reflection::Schema*
SchemaCache::GetSchemaOfObject(const string& sObjName) const
{
	auto itr = m_mapObjName2Bfbs.find(sObjName);
	if (itr == m_mapObjName2Bfbs.end()) return nullptr;
	const BfbsSptr& pBfbs = (*itr).second;
	assert(pBfbs);
	return flatbuffers::GetRoot<reflection::Schema>(pBfbs.get());
}

