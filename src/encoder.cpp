#include "encoder.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

#include <LuaIntf/LuaIntf.h>

#include <unordered_map>

using flatbuffers::uoffset_t;
using std::string;

Encoder::Encoder(const reflection::Schema& schema) :
	m_schema(schema),
	m_vObjects(*schema.objects())
{
}

bool Encoder::Encode(const string& sName, const LuaRef& luaTable)
{
	Reset();

	const Object* pObj = m_vObjects.LookupByKey(sName.c_str());
	assert(pObj);
	m_nameStack.Push(sName);
	uoffset_t offset = EncodeObject(*pObj, luaTable);
	m_nameStack.SafePop();
	if (!offset)  // An offset of 0 means error.
		return false;

	m_fbb.Finish(flatbuffers::Offset<void>(offset));  // Todo: Add file_identifier if root_type
	return true;
}

string Encoder::GetResultStr() const
{
	const char* pBuffer = reinterpret_cast<const char*>(
		m_fbb.GetBufferPointer());
	return string(pBuffer, m_fbb.GetSize());
}

// Todo: check required fields.
// Todo: Skip default value.

uoffset_t Encoder::EncodeObject(const Object& obj, const LuaRef& luaTable)
{
	return obj.is_struct() ?
		EncodeStruct(obj, luaTable) :
		EncodeTable(obj, luaTable);
}

uoffset_t Encoder::EncodeStruct(const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	const auto& vFields = *obj.fields();
	(void)m_fbb.StartStruct(obj.minalign());

	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!CheckObjectField(pField, sKey))
			return 0;

		LuaRef value = e.value<LuaRef>();
		const reflection::Type& type = *pField->type();
		// Todo: check type of value...

		reflection::BaseType eBaseType = type.base_type();
		if (eBaseType <= reflection::Double)
		{
			AddElement(*pField, value);  // XXX throw?
			continue;
		}

		assert(eBaseType == reflection::Obj);
		const Object* pFieldObj = m_vObjects[type.index()];
		assert(pFieldObj);
		assert(pFieldObj->is_struct());
		m_nameStack.Push(sKey);
		if (0 == EncodeStruct(*pFieldObj, value))
			return 0;
		m_nameStack.SafePop();
	}
	return m_fbb.EndStruct();
}

uoffset_t Encoder::EncodeTable(const Object& obj, const LuaRef& luaTable)
{
	assert(!obj.is_struct());
	// Cache to map before StartTable().
	Field2Scalar mapScalar;
	Field2Offset mapOffset;
	if (!CacheFields(obj, luaTable, mapScalar, mapOffset))
		return 0;

	uoffset_t start = m_fbb.StartTable();
	AddElements(mapScalar);
	AddOffsets(mapOffset);
	return m_fbb.EndTable(start, obj.fields()->size());
}

uoffset_t Encoder::EncodeVector(const Object& elemObj, const LuaRef& luaArray)
{
	// todo: check luaArray is array
	return 0;
}

// Cache fields to 2 maps.
bool Encoder::CacheFields(const Object& obj, const LuaRef& luaTable,
	Field2Scalar& rMapScalar, Field2Offset& rMapOffset)
{
	const auto& vFields = *obj.fields();
	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!CheckObjectField(pField, sKey))
			return false;
		assert(pField);

		LuaRef value = e.value<LuaRef>();
		CacheField(pField, value, rMapScalar, rMapOffset);
	}
	return true;
}

// Cache field to 2 maps.
void Encoder::CacheField(const Field* pField, const LuaRef& luaValue,
	Field2Scalar& rMapScalar, Field2Offset& rMapOffset)
{
	assert(pField);
	const reflection::Type& type = *pField->type();
	// Todo: check type of value...
	switch (type.base_type())
	{
	case reflection::String:
		rMapOffset[pField] = m_fbb.CreateString(
			luaValue.toValue<const char*>()).o;
		break;
	case reflection::Vector:
		rMapOffset[pField] = EncodeVector(
			*m_vObjects[type.index()], luaValue);
		break;
	case reflection::Obj:
		// Todo: is_struct...
		rMapOffset[pField] = EncodeObject(
			*m_vObjects[type.index()], luaValue);
		break;
	case reflection::Union:
		// XXX
		break;
	default:
		rMapScalar[pField] = luaValue;
		break;
	}  // switch
}

void Encoder::AddElements(const Field2Scalar& mapScalar)
{
	for (const auto& e : mapScalar)
	{
		const Field* pField = e.first;
		assert(pField);
		AddElement(*pField, e.second);
	}
}

void Encoder::AddElement(const Field& field, const LuaRef& elementValue)
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
		const Field* pField = e.first;
		assert(pField);
		uoffset_t offset = e.second;
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

string Encoder::PopFullFieldName(const string& sFieldName)
{
	return m_nameStack.PopFullFieldName(sFieldName);
}

void Encoder::Reset()
{
	m_fbb.Clear();
	m_sError.clear();
	m_nameStack.Reset();
}

// Set error and return false if field is illegal.
// sFieldName is only used for error message.
bool Encoder::CheckObjectField(const Field* pField, const string& sFieldName)
{
	if (!pField)
	{
		m_sError = "illegal field " + PopFullFieldName(sFieldName);
		return false;
	}
	if (pField->deprecated())
	{
		m_sError = "deprecated field " + PopFullFieldName(sFieldName);
		return false;
	}
	return true;
}

