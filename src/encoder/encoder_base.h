#ifndef LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_
#define LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_

#include "encoder_context.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/reflection.h>  // for objects()
#include <LuaIntf/LuaIntf.h>  // for createTable()

#include <sstream>  // for ostringstream
#include <string>

#define ERR_RET(ErrorStr) do { \
	SetError(ErrorStr); \
	return; \
} while(0)

static inline bool IsLuaNumber(const LuaIntf::LuaRef& luaValue)
{
	return luaValue.type() == LuaIntf::LuaTypeID::NUMBER;
}

class EncoderBase
{
public:
	explicit EncoderBase(EncoderContext& rCtx) : m_rCtx(rCtx) {};
	virtual ~EncoderBase() {};

public:
	using LuaRef = LuaIntf::LuaRef;
	using string = std::string;

protected:
	inline void CheckScalarLuaValue(const LuaRef& luaValue);

	template <typename T>
	T LuaToNumber(const LuaRef& luaValue);

protected:
	const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>&
	Objects() const
	{
		return *m_rCtx.schema.objects();
	}
	flatbuffers::FlatBufferBuilder& Builder() { return m_rCtx.fbb; }
	const flatbuffers::FlatBufferBuilder& Builder() const { return m_rCtx.fbb; }

protected:
	bool Bad() const { return !m_rCtx.sError.empty(); }
	void SetError(const string& sError);
	void PushName(const string& sName) { m_rCtx.nameStack.Push(sName); }
	void PushName(const reflection::Object& object)
	{
		PushName(object.name()->c_str());
	}
	void PushName(const reflection::Field& field)
	{
		PushName(field.name()->c_str());
	}
	void SafePopName() { m_rCtx.nameStack.SafePop(); }
	string PopFullName();
	string PopFullFieldName(const string& sFieldName);
	string PopFullFieldName(const reflection::Field& field);
	string PopFullVectorName(size_t index);

private:
	template <typename T>
	void CheckNumberRange(double dValue, const LuaRef& luaValue);

	// Figure out if int or float.
	bool IsInteger(double dValue) const;

protected:
	EncoderContext& m_rCtx;
};  // class EncoderBase

void EncoderBase::CheckScalarLuaValue(const LuaRef& luaValue)
{
	if (IsLuaNumber(luaValue)) return;
	SetError("scalar field " + PopFullName() + " is " + luaValue.typeName());
}

template <typename T>
T EncoderBase::LuaToNumber(const LuaRef& luaValue)
{
	static_assert(std::is_scalar<T>::value,
		"LuaToNumber() is only for scalar types.");
	CheckScalarLuaValue(luaValue);
	if (Bad()) return T();

	double dVal = luaValue.toValue<double>();
	if (!IsInteger(dVal))
	{
		SetError("integer field " + PopFullName() + " is "
			+ luaValue.toValue<string>());
		return T();
	}

	CheckNumberRange<T>(dVal, luaValue);
	if (Bad()) return T();

	if (sizeof(T) >= sizeof(int64_t))
		return luaValue.toValue<T>();
	assert(static_cast<T>(dVal) == luaValue.toValue<T>());
	return static_cast<T>(dVal);
}

template <> float EncoderBase::LuaToNumber<float>(const LuaRef& luaValue);
template <> double EncoderBase::LuaToNumber<double>(const LuaRef& luaValue);

template <typename T>
void EncoderBase::CheckNumberRange(double dValue, const LuaRef& luaValue)
{
	assert(IsLuaNumber(luaValue));
	assert(dValue == luaValue.toValue<double>());

	if (dValue >= std::numeric_limits<T>::min() &&
		dValue <= std::numeric_limits<T>::max())
		return;

	std::ostringstream oss;
	oss << "field " << PopFullName() << "("
		<< luaValue.toValue<string>()
		<< ") is out of range [";
	if (sizeof(T) <= sizeof(int8_t))  // or uint8_t
	{
		oss << static_cast<int>(std::numeric_limits<T>::min()) << ", "
			<< static_cast<int>(std::numeric_limits<T>::max()) << "]";
	}
	else
	{
		oss << std::numeric_limits<T>::min() << ", "
			<< std::numeric_limits<T>::max() << "]";
	}
	SetError(oss.str());
}

#endif  // LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_
