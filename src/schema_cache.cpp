#include "schema_cache.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

SchemaCache::SchemaCache() {}
SchemaCache::~SchemaCache() {}

std::tuple<bool, std::string>
SchemaCache::LoadBfbsFile(const std::string& sBfbsFile)
{
	return std::make_tuple(false, "to be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadBfbs(const std::string& sBfbs)
{
	// Verify it.
	flatbuffers::Verifier verifier(
		reinterpret_cast<const uint8_t *>(sBfbs.data()), sBfbs.length());
	if (!reflection::VerifySchemaBuffer(verifier))
		return std::make_tuple(false, "failed to verify schema buffer");

	BfbsSptr pBfbs(new char[sBfbs.length()]);
	memcpy(pBfbs.get(), sBfbs.data(), sBfbs.length());
	const Schema* pSchema = flatbuffers::GetRoot<Schema>(pBfbs.get());
	assert(pSchema);
	for (const auto* pObj : pSchema->objects())
	{
		assert(pObj);
		const std::string& sName = pObj->name().str();
		if (m_mapObjName2Bfbs[sName])
		{
			std::string sError = "2 objects name '" + sName + "'";
			return std::make_tuple(false, sError);
		}
		m_mapObjName2Bfbs[sName] = pBfbs;
	}

	return std::make_tuple(true, "");
}

std::tuple<bool, std::string>
SchemaCache::LoadFbsFile(const std::string& sFbsFile)
{
	return std::make_tuple(false, "to be implemented");
}

std::tuple<bool, std::string>
SchemaCache::LoadFbs(const std::string& sFbs)
{
	return std::make_tuple(false, "to be implemented");
}

const reflection::Schema*
SchemaCache::GetSchemaOfObject(const std::string& sObjName) const
{
	auto itr = m_mapObjName2Bfbs.find(sObjName);
	if (itr == m_mapObjName2Bfbs.end()) return nullptr;
	const BfbsSptr& pBfbs = (*itr).second;
	assert(pBfbs);
	return flatbuffers::GetRoot<reflection::Schema>(pBfbs.get());
}

