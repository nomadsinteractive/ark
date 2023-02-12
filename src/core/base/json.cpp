#include "core/base/json.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/inf/array.h"
#include "core/inf/writable.h"

#include "core/util/log.h"

namespace ark {

struct Json::Stub {
    Stub() {
    }
    Stub(nlohmann::json json)
        : _json(std::move(json)) {
    }

    template<typename T> sp<Array<T>> toArray() const {
        std::vector<T> array(_json.size());
        for(size_t i = 0; i < _json.size(); ++i)
            array.push_back(_json.at(i).get<T>());
        return sp<typename Array<T>::Vector>::make(std::move(array));
    }

    nlohmann::json _json;
};

Json::Json(const sp<Json>& other)
    : _stub(sp<Stub>::make())
{
    if(other)
        _stub->_json = other->_stub->_json;
}

Json::Json(const String& value)
{
    _stub->_json = value.c_str();
}

Json::Json(int32_t value)
    : _stub(sp<Stub>::make())
{
    _stub->_json = value;
}

Json::Json(float value)
    : _stub(sp<Stub>::make())
{
    _stub->_json = value;
}

Json::Json(const sp<ByteArray>& value)
{
    const size_t length = value->size();
    std::vector<uint8_t> binary(length);
    memcpy(binary.data(), value->buf(), length);
    _stub->_json = nlohmann::json::binary(binary);
}

Json::Json(const sp<IntArray>& value)
    : _stub(sp<Stub>::make())
{
    const int32_t* buf = value->buf();
    const size_t length = value->length();
    for(size_t i = 0; i < length; ++i)
        _stub->_json.emplace_back(buf[i]);
}

Json::Json(const sp<FloatArray>& value)
{
    const float* buf = value->buf();
    const size_t length = value->length();
    for(size_t i = 0; i < length; ++i)
        _stub->_json.emplace_back(buf[i]);
}

Json::Json(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

Json::~Json()
{
}

void Json::load(const String& content)
{
    _stub->_json = nlohmann::json::parse(content.c_str());
}

void Json::loadFromFile(const String& filename)
{
    std::ifstream input(filename.c_str());
    input >> _stub->_json;
}

String Json::getString(const String& key, const String& defValue) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return defValue;
    CHECK(val->is_string(), "Key \"%s\" value is not a string", key.c_str());
    return val->get<std::string>().c_str();
}

int32_t Json::getInt(const String& key, int32_t defValue) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return defValue;
    CHECK(val->is_number_integer(), "Key \"%s\" value is not an integer", key.c_str());
    return val->get<int32_t>();
}

float Json::getFloat(const String& key, float defValue) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return defValue;
    CHECK(val->is_number_float() || val->is_number_integer(), "Key \"%s\" value is not float", key.c_str());
    return val->get<float>();
}

void Json::setString(const String& key, const String& value) const
{
    _stub->_json[key.c_str()] = value.c_str();
}

void Json::setInt(const String& key, int32_t value) const
{
    _stub->_json[key.c_str()] = value;
}

void Json::setFloat(const String& key, float value) const
{
    _stub->_json[key.c_str()] = value;
}

String Json::toString() const
{
    return _stub->_json.get<std::string>();
}

int32_t Json::toInt() const
{
    return _stub->_json.get<int32_t>();
}

float Json::toFloat() const
{
    return _stub->_json.get<float>();
}

bytearray Json::toByteArray() const
{
    DASSERT(isArray());
    return _stub->toArray<uint8_t>();
}

intarray Json::toIntArray() const
{
    DASSERT(isArray());
    return _stub->toArray<int32_t>();
}

floatarray Json::toFloatArray() const
{
    DASSERT(isArray());
    return _stub->toArray<float>();
}

sp<Json> Json::get(const String& key) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return nullptr;
    return sp<Json>::make(sp<Stub>::make(*val));
}

void Json::set(const String& key, const Json& value) const
{
    _stub->_json[key.c_str()] = value._stub->_json;
}

sp<Json> Json::at(int32_t index) const
{
    return sp<Json>::make(sp<Stub>::make(_stub->_json.at(static_cast<size_t>(index))));
}

void Json::append(const Json& value)
{
    _stub->_json.push_back(value._stub->_json);
}

void Json::append(int32_t value)
{
    _stub->_json.push_back(value);
}

void Json::append(float value)
{
    _stub->_json.push_back(value);
}

bool Json::isArray() const
{
    return _stub->_json.is_array();
}

bool Json::isFloat() const
{
    return _stub->_json.is_number_float();
}

bool Json::isInteger() const
{
    return _stub->_json.is_number_integer() || _stub->_json.is_number_unsigned();
}

bool Json::isString() const
{
    return _stub->_json.is_string();
}

bool Json::isObject() const
{
    return _stub->_json.is_object();
}

size_t Json::size() const
{
    return _stub->_json.size();
}

void Json::foreach(const std::function<bool (const Json&)>& callback)
{
    Json item;
    for(size_t i = 0; i < size(); ++i)
    {
        item._stub->_json = _stub->_json.at(i);
        if(!callback(item))
            break;
    }
}

sp<ByteArray> Json::toBson() const
{
    return sp<ByteArray::Vector>::make(nlohmann::json::to_bson(_stub->_json));
}

Json Json::fromBson(ByteArray& content)
{
    return sp<Stub>::make(nlohmann::json::from_bson(content.buf(), content.buf() + content.size(), true, true));
}

String Json::dump() const
{
    return _stub->_json.dump();
}

}
