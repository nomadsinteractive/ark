#include "renderer/base/uniform.h"

#include "core/epi/changed.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/util/strings.h"

namespace ark {

Uniform::Uniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed)
    : _name(name), _type(type), _flatable(flatable), _notifier(changed)
{
    DWARN(changed, "Uniform: %s has no notifier, it's probably not a good idea", _name.c_str());
}

const String& Uniform::name() const
{
    return _name;
}

Uniform::Type Uniform::type() const
{
    return _type;
}

Uniform::Type Uniform::toType(const String& declaredType)
{
    if(declaredType == "int")
        return TYPE_I1;
    if(declaredType == "float")
        return TYPE_F1;
    if(declaredType == "vec2")
        return TYPE_F2;
    if(declaredType == "vec3")
        return TYPE_F3;
    if(declaredType == "vec4")
        return TYPE_F4;
    if(declaredType == "mat3")
        return TYPE_MAT3;
    if(declaredType == "mat4")
        return TYPE_MAT4;
    if(declaredType == "sampler2D")
        return TYPE_SAMPLER2D;
    DFATAL("Unknow uniform type: %s", declaredType.c_str());
    return TYPE_F1;
}

const sp<Flatable>& Uniform::flatable() const
{
    return _flatable;
}

const sp<Changed>& Uniform::notifier() const
{
    return _notifier;
}

String Uniform::declaration() const
{
    String t;
    uint32_t s = 0;
    switch(_type) {
    case TYPE_I1V:
        s = _flatable->size() / 4;
    case TYPE_I1:
        t = "int";
        break;
    case TYPE_F1V:
        s = _flatable->size() / 4;
    case TYPE_F1:
        t = "float";
        break;
    case TYPE_F2V:
        s = _flatable->size() / 8;
    case TYPE_F2:
        t = "vec2";
        break;
    case TYPE_F3V:
        s = _flatable->size() / 12;
    case TYPE_F3:
        t = "vec3";
        break;
    case TYPE_F4V:
        s = _flatable->size() / 16;
    case TYPE_F4:
        t = "vec4";
        break;
    case TYPE_MAT4V:
        s = _flatable->size() / 64;
    case TYPE_MAT4:
        t = "mat4";
        break;
    case TYPE_MAT3V:
        s = _flatable->size() / 36;
    case TYPE_MAT3:
        t = "mat3";
        break;
    case TYPE_SAMPLER2D:
        t = "sampler2D";
        break;
    }
    return s ? Strings::sprintf("uniform %s %s[%d];", t.c_str(), _name.c_str(), s + 1) : Strings::sprintf("uniform %s %s;", t.c_str(), _name.c_str());
}

void Uniform::notify() const
{
    _notifier->notify();
}

}
