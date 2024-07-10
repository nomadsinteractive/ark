#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Json final {
public:
    struct Stub;

//  [[script::bindings::auto]]
    Json(const sp<Json>& other = nullptr);
//  [[script::bindings::auto]]
    Json(const String& value);
//  [[script::bindings::auto]]
    Json(int32_t value);
//  [[script::bindings::auto]]
    Json(float value);
//  [[script::bindings::auto]]
    Json(const sp<ByteArray>& value);
//  [[script::bindings::auto]]
    Json(const sp<IntArray>& value);
//  [[script::bindings::auto]]
    Json(const sp<FloatArray>& value);

    Json(sp<Stub> stub);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Json);
    ~Json();

//  [[script::bindings::auto]]
    void load(const String& content);
//  [[script::bindings::auto]]
    void loadFromFile(const String& filename);

//  [[script::bindings::auto]]
    String getString(const String& key, const String& defValue = "") const;
//  [[script::bindings::auto]]
    int32_t getInt(const String& key, int32_t defValue = 0) const;
//  [[script::bindings::auto]]
    float getFloat(const String& key, float defValue = 0) const;

//  [[script::bindings::auto]]
    void setString(const String& key, const String& value) const;
//  [[script::bindings::auto]]
    void setInt(const String& key, int32_t value) const;
//  [[script::bindings::auto]]
    void setFloat(const String& key, float value) const;

//  [[script::bindings::auto]]
    String toString() const;
//  [[script::bindings::auto]]
    int32_t toInt() const;
//  [[script::bindings::auto]]
    float toFloat() const;

//  [[script::bindings::auto]]
    bytearray toByteArray() const;
//  [[script::bindings::auto]]
    intarray toIntArray() const;
//  [[script::bindings::auto]]
    floatarray toFloatArray() const;

//  [[script::bindings::auto]]
    sp<Json> get(const String& key) const;

//  [[script::bindings::auto]]
    void set(const String& key, const Json& value) const;

//  [[script::bindings::auto]]
    sp<Json> at(int32_t index) const;

//  [[script::bindings::auto]]
    void append(const Json& value);
//  [[script::bindings::auto]]
    void append(int32_t value);
//  [[script::bindings::auto]]
    void append(float value);

//  [[script::bindings::auto]]
    bool isArray() const;
//  [[script::bindings::auto]]
    bool isFloat() const;
//  [[script::bindings::auto]]
    bool isInteger() const;
//  [[script::bindings::auto]]
    bool isString() const;
//  [[script::bindings::auto]]
    bool isObject() const;

//  [[script::bindings::auto]]
    size_t size() const;

    void foreach(const std::function<bool(const Json&)>& callback);

//  [[script::bindings::auto]]
    sp<ByteArray> toBson() const;
//  [[script::bindings::auto]]
    static Json fromBson(ByteArray& content);

//  [[script::bindings::auto]]
    String dump() const;

private:
    sp<Stub> _stub;
};

}
