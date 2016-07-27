#include "encoder.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

#include <LuaIntf/LuaIntf.h>

#include <unordered_map>

Encoder::Encoder(const reflection::Schema& schema) :
	m_schema(schema),
	m_vObjects(*schema.objects())
{
}

bool Encoder::Encode(const std::string& sName, const LuaRef& table)
{
	Reset();

	const Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	flatbuffers::uoffset_t offset = Encode(*pObj, table);
	if (!offset)  // An offset of 0 means error.
		return false;

	m_fbb.Finish(flatbuffers::Offset<void>(offset));  // Todo: Add file_identifier if root_type
	return true;
}

std::string Encoder::GetResultStr() const
{
	const char* pBuffer = reinterpret_cast<const char*>(
		m_fbb.GetBufferPointer());
	return std::string(pBuffer, m_fbb.GetSize());
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
	return m_fbb.EndTable(start, obj.fields()->size());
}

// Cache fields to 2 maps.
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
			m_sError = "illegal field " + PopFullFieldName(sKey);
			return false;
		}
		if (pField->deprecated())
		{
			m_sError = "deprecated field " + PopFullFieldName(sKey);
			return false;
		}

		LuaRef value = e.value<LuaRef>();
		const reflection::Type& type = *pField->type();
		// Todo: check type of value...
		switch (type.base_type())
		{
		case reflection::String:
			rMapOffset[pField] = m_fbb.CreateString(
				value.toValue<const char*>()).o;
			break;
		case reflection::Vector:
			// Todo: check key (may be map)
			// XXX
			break;
		case reflection::Obj:
			assert(type.index() >= 0);
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

template <typename ElementType, typename DefaultValueType>
inline void Encoder::AddElement(uint16_t offset,
	const LuaRef& elementValue, DefaultValueType defaultValue)
{
	m_fbb.AddElement(offset,
		elementValue.toValue<ElementType>(),
		static_cast<ElementType>(defaultValue));
}

std::string Encoder::PopFullFieldName(const std::string& sFieldName)
{
	return m_nameStack.PopFullFieldName(sFieldName);
}

void Encoder::Reset()
{
	m_fbb.Clear();
	m_sError.clear();

	NameStack emptyStack;
	m_nameStack.swap(emptyStack);  // swap to clear
}

