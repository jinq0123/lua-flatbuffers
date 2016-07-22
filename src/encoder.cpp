#include "encoder.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

#include <LuaIntf/LuaIntf.h>

#include <unordered_map>

Encoder::Encoder(const reflection::Schema& schema) :
	m_schema(schema),
	m_vObjects(*schema.objects())
{
}

std::tuple<bool, std::string>
Encoder::Encode(const std::string& sName, const LuaRef& table)
{
	Reset();

	const Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	if (!Encode(*pObj, table))  // An offset of 0 means error.
		return std::make_tuple(false, m_sError);

	const char* pBuffer = reinterpret_cast<const char*>(
		m_fbb.GetBufferPointer());
	return std::make_tuple(true, std::string(pBuffer, m_fbb.GetSize()));
}

// Todo: check required fields.
// Todo: Skip default value.

flatbuffers::uoffset_t
Encoder::Encode(const Object& obj, const LuaRef& table)
{
	m_nameStack.push(obj.name()->str());

	// Cache to map before StartTable().
	Field2Scalar mapScalar;
	Field2Offset mapOffset;
	if (!CacheFields(obj, table, mapScalar, mapOffset))
		return 0;

	flatbuffers::uoffset_t start = m_fbb.StartTable();
	AddElements(mapScalar);
	AddOffsets(mapOffset);
	m_nameStack.pop();
	// XXX numFields = vFields.size()?
	return m_fbb.EndTable(start, obj.fields()->size());
}

bool Encoder::CacheFields(const Object& obj, const LuaRef& table,
	Field2Scalar& rMapScalar, Field2Offset& rMapOffset)
{
	const auto& vFields = *obj.fields();
	// bool isStruct = obj.is_struct();

	for (const auto& e : table)
	{
		std::string sKey = e.key<std::string>();
		const reflection::Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField)
		{
			m_sError = "illegal field " + GetFullFieldName(sKey);
			return false;
		}
		if (pField->deprecated())
		{
			m_sError = "deprecated field " + GetFullFieldName(sKey);
			return false;
		}

		LuaRef value = e.value<LuaRef>();
		const reflection::Type& type = *pField->type();
		switch (type.base_type())
		{
		case reflection::String:
			// XXX
			break;
		case reflection::Vector:
			// XXX
			break;
		case reflection::Obj:
			rMapOffset[pField] = Encode(*m_vObjects[type.index()], value);
			break;
		case reflection::Union:
			// XXX
			break;
		default:
			rMapScalar[pField] = value;
			break;
		}
	}
	return true;
}

void Encoder::AddElements(const Field2Scalar& mapScalar)
{
	for (const auto& e : mapScalar)
	{
		const reflection::Field* pField = e.first;
		assert(pField);
		AddElement(*pField, e.second);
	}
}

void Encoder::AddElement(const reflection::Field& field,
	const LuaRef& elementValue)
{
	const reflection::Type& type = *field.type();
	int64_t defInt = field.default_integer();
	double defReal = field.default_real();
	uint16_t offset = field.offset();

	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		AddElement<uint8_t>(offset, elementValue, defInt);
		break;
	case reflection::Byte:
		AddElement<int8_t>(offset, elementValue, defInt);
		break;
	case reflection::Short:
		AddElement<int16_t>(offset, elementValue, defInt);
		break;
	case reflection::UShort:
		AddElement<uint16_t>(offset, elementValue, defInt);
		break;
	case reflection::Int:
		AddElement<int32_t>(offset, elementValue, defInt);
		break;
	case reflection::UInt:
		AddElement<uint32_t>(offset, elementValue, defInt);
		break;
	case reflection::Long:
		AddElement<int64_t>(offset, elementValue, defInt);
		break;
	case reflection::ULong:
		AddElement<uint64_t>(offset, elementValue, defInt);
		break;
	case reflection::Float:
		AddElement<float>(offset, elementValue, defReal);
		break;
	case reflection::Double:
		AddElement<double>(offset, elementValue, defReal);
		break;
	default:
		assert(false);
		break;
	}
}

void Encoder::AddOffsets(const Field2Offset& mapOffset)
{
	for (const auto& e : mapOffset)
	{
		const reflection::Field* pField = e.first;
		assert(pField);
		flatbuffers::uoffset_t offset = e.second;
		m_fbb.AddOffset(pField->offset(), flatbuffers::Offset<void>(offset));
	}
}

template <typename T>
inline void Encoder::AddElement(uint16_t offset,
	const LuaRef& elementValue, T defaultValue)
{
	m_fbb.AddElement<T>(offset, elementValue.toValue<T>(), defaultValue);
}

std::string Encoder::GetFullFieldName(const std::string& sFieldName) const
{
	NameStack ns(m_nameStack);
	std::string sResult(sFieldName);
	while (!ns.empty())
	{
		sResult.insert(0, ns.top() + ".");
		ns.pop();
	}
	return sResult;
}

void Encoder::Reset()
{
	m_fbb.Clear();
	m_sError.clear();

	NameStack emptyStack;
	m_nameStack.swap(emptyStack);  // swap to clear
}

