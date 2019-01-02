#include "renderer/base/uniform.h"

#include "core/epi/changed.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/util/strings.h"

namespace ark {

Uniform::Uniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& dirty, int32_t binding)
    : _name(name), _type(type), _flatable(flatable), _dirty(dirty), _binding(binding)
{
}

const String& Uniform::name() const
{
    return _name;
}

Uniform::Type Uniform::type() const
{
    return _type;
}

int32_t Uniform::binding() const
{
    return _binding;
}

void Uniform::setBinding(int32_t binding)
{
    _binding = binding;
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

void Uniform::toTypeLength(String& type, uint32_t& length) const
{
    switch(_type) {
    case TYPE_I1V:
        length = _flatable->size() / 4;
    case TYPE_I1:
        type = "int";
        break;
    case TYPE_F1V:
        length = _flatable->size() / 4;
    case TYPE_F1:
        type = "float";
        break;
    case TYPE_F2V:
        length = _flatable->size() / 8;
    case TYPE_F2:
        type = "vec2";
        break;
    case TYPE_F3V:
        length = _flatable->size() / 12;
    case TYPE_F3:
        type = "vec3";
        break;
    case TYPE_F4V:
        length = _flatable->size() / 16;
    case TYPE_F4:
        type = "vec4";
        break;
    case TYPE_MAT4V:
        length = _flatable->size() / 64;
    case TYPE_MAT4:
        type = "mat4";
        break;
    case TYPE_MAT3V:
        length = _flatable->size() / 36;
    case TYPE_MAT3:
        type = "mat3";
        break;
    case TYPE_SAMPLER2D:
        type = "sampler2D";
        break;
    default:
        DFATAL("Unsupported type: %d", _type);
    }
}

const sp<Flatable>& Uniform::flatable() const
{
    return _flatable;
}

void Uniform::setFlatable(const sp<Flatable>& flatable)
{
    _flatable = flatable;
}

void Uniform::setObserver(const sp<Boolean>& dirty)
{
    if(_dirty)
        _dirty->set(dirty);
    else
        _dirty = sp<Changed>::make(dirty);
}

bool Uniform::dirty() const
{
    return _dirty ? _dirty->dirty() : true;
}

String Uniform::declaration() const
{
    String t;
    uint32_t s = 0;
    toTypeLength(t, s);
    return s ? Strings::sprintf("uniform %s %s[%d];", t.c_str(), _name.c_str(), s + 1) : Strings::sprintf("uniform %s %s;", t.c_str(), _name.c_str());
}

void Uniform::notify() const
{
    if(_dirty)
        _dirty->notify();
}

}
