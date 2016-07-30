#include "encoder.h"

#include <flatbuffers/reflection_generated.h>  // for Schema

#include <LuaIntf/LuaIntf.h>

#include <unordered_map>

#define ERR_RET_FALSE(ErrorStr) do { \
	m_sError = ErrorStr; \
	return false; \
} while(0)

using flatbuffers::uoffset_t;
using std::string;

Encoder::Encoder(const reflection::Schema& schema) :
	m_schema(schema),
	m_vObjects(*schema.objects())
{
}

bool Encoder::Encode(const string& sName, const LuaRef& luaTable)
{
	if (!luaTable.isTable())
		ERR_RET_FALSE("lua data is not table");

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
	(void)m_fbb.StartStruct(obj.minalign());
	uint8_t* pBuf = m_fbb.ReserveElements(obj.bytesize(), 1);
	assert(pBuf);
	if (!EncodeStructToBuf(obj, luaTable, pBuf))
		return 0;
	return m_fbb.EndStruct();
}

bool Encoder::EncodeStructToBuf(const Object& obj,
	const LuaRef& luaTable, uint8_t* pBuf)
{
	assert(pBuf);
	// Struct should traverse all fields of object.
	// Lua table traverse is better, to check fields count.
	for (const Field* pField : *obj.fields())
	{
		assert(pField);
		assert(!pField->deprecated());  // Struct has no deprecated field.
		if (!EncodeStructFieldToBuf(*pField, luaTable, pBuf))
			return false;
	}  // for
	return true;
}  // EncodeStructToBuf()

bool Encoder::CheckStructFields(const Object& obj, const LuaRef& luaTable)
{
	assert(obj.is_struct());
	const auto& vFields = *obj.fields();
	for (const auto& e : luaTable)
	{
		string sKey = e.key<string>();
		const Field* pField = vFields.LookupByKey(sKey.c_str());
		if (!pField)
			ERR_RET_FALSE("illegal field " + PopFullFieldName(sKey));
		assert(!pField->deprecated());
	}
	return true;
}  // CheckStructFields()

bool Encoder::EncodeStructFieldToBuf(const Field& field,
	const LuaRef& luaTable, uint8_t* pBuf)
{
	assert(pBuf);
	const char* pFieldName = field.name()->c_str();
	const LuaRef luaValue = luaTable.get(pFieldName);
	if (!luaValue)
		ERR_RET_FALSE("missing struct field " + PopFullFieldName(pFieldName));

	const reflection::Type& type = *field.type();
	// Todo: check type of value...

	uint16_t offset = field.offset();
	uint8_t* pDest = pBuf + offset;
	reflection::BaseType eBaseType = type.base_type();
	if (eBaseType <= reflection::Double)
	{
		EncodeStructElementToBuf(eBaseType, luaValue, pBuf + offset);  // XXX throw?
		return true;
	}

	assert(eBaseType == reflection::Obj);
	const Object* pFieldObj = m_vObjects[type.index()];
	assert(pFieldObj);
	assert(pFieldObj->is_struct());
	m_nameStack.Push(pFieldName);
	if (!EncodeStructToBuf(*pFieldObj, luaValue, pDest))
		return false;
	m_nameStack.SafePop();
	return true;
}  // EncodeStructFieldToBuf()

template <typename T>
static void CopyToBuf(const LuaIntf::LuaRef& luaValue, uint8_t* pDest)
{
	T val = luaValue.toValue<T>();  // Todo: throw?
	*reinterpret_cast<T*>(pDest) = val;
}

// Encode struct scalar element to buffer.
void Encoder::EncodeStructElementToBuf(reflection::BaseType eType,
	const LuaRef& luaValue, uint8_t* pDest)
{
	assert(pDest);
	switch (eType)
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		CopyToBuf<uint8_t>(luaValue, pDest);
		break;
	case reflection::Byte:
		CopyToBuf<int8_t>(luaValue, pDest);
		break;
	case reflection::Short:
		CopyToBuf<int16_t>(luaValue, pDest);
		break;
	case reflection::UShort:
		CopyToBuf<uint16_t>(luaValue, pDest);
		break;
	case reflection::Int:
		CopyToBuf<int32_t>(luaValue, pDest);
		break;
	case reflection::UInt:
		CopyToBuf<uint32_t>(luaValue, pDest);
		break;
	case reflection::Long:
		CopyToBuf<int64_t>(luaValue, pDest);
		break;
	case reflection::ULong:
		CopyToBuf<uint64_t>(luaValue, pDest);
		break;
	case reflection::Float:
		CopyToBuf<float>(luaValue, pDest);
		break;
	case reflection::Double:
		CopyToBuf<double>(luaValue, pDest);
		break;
	}
	assert(!"Illegal type.");
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

uoffset_t Encoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	assert(type.base_type() == reflection::Vector);
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
		rMapOffset[pField] = EncodeVector(type, luaValue);
		break;
	case reflection::Obj:
	{
		const Object* pObj = m_vObjects[type.index()];
		assert(pObj);
		if (pObj->is_struct()) rMapScalar[pField] = luaValue;
		else rMapOffset[pField] = EncodeObject(*pObj, luaValue);
		break;
	}
	case reflection::Union:
		// XXX get union underlying type...
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
	case reflection::Obj:
		// XXX struct...
		break;
	default:
		assert(!"Illegal type.");
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
		ERR_RET_FALSE("illegal field " + PopFullFieldName(sFieldName));
	if (pField->deprecated())
		ERR_RET_FALSE("deprecated field " + PopFullFieldName(sFieldName));
	return true;
}

