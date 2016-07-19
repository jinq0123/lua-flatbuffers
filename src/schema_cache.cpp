#include "schema_cache.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

SchemaCache::SchemaCache() {}
SchemaCache::~SchemaCache() {}

std::tuple<bool, std::string>
SchemaCache::LoadBfbsFile(const string& sBfbsFile)
{
	return std::make_tuple(false, "to be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadBfbs(const string& sBfbs)
{
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
		if (m_mapObjName2Bfbs[sName])
		{
			string sError = "2 objects name '" + sName + "'";
			return std::make_tuple(false, sError);
		}
		m_mapObjName2Bfbs[sName] = pBfbs;
	}

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

