#include "core/base/json.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "core/base/string.h"
#include "core/inf/array.h"

namespace ark {

struct Json::Stub {
    Stub() {
    }
    Stub(nlohmann::json json)
        : _json(std::move(json)) {
    }

    nlohmann::json _json;
};

Json::Json()
    : _stub(sp<Stub>::make())
{
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
    DCHECK(val->is_string(), "Key \"%s\" value is not a string", key.c_str());
    return val->get<std::string>().c_str();
}

int32_t Json::getInt(const String& key, int32_t defValue) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return defValue;
    DCHECK(val->is_number_integer(), "Key \"%s\" value is not an integer", key.c_str());
    return val->get<int32_t>();
}

float Json::getFloat(const String& key, float defValue) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return defValue;
    DCHECK(val->is_number_float(), "Key \"%s\" value is not float", key.c_str());
    return val->get<float>();
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

sp<Json> Json::get(const String& key) const
{
    const auto val = _stub->_json.find(key.c_str());
    if(val == _stub->_json.end())
        return nullptr;
    return sp<Json>::make(sp<Stub>::make(*val));
}

sp<Json> Json::at(int32_t index) const
{
    return sp<Json>::make(sp<Stub>::make(_stub->_json.at(index)));
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

bytearray Json::toBson() const
{
    return sp<ByteArray::Vector>::make(nlohmann::json::to_bson(_stub->_json));
}

String Json::dump() const
{
    return _stub->_json.dump();
}

}
