#include "renderer/base/uniform.h"

#include "core/base/observer.h"
#include "core/base/notifier.h"
#include "core/inf/array.h"
#include "core/inf/input.h"
#include "core/inf/variable.h"
#include "core/util/boolean_type.h"
#include "core/util/strings.h"

namespace ark {

Uniform::Uniform(const String& name, const String& declaredType, Uniform::Type type, size_t size, uint32_t length, const sp<Input>& flatable, int32_t binding)
    : _name(name), _declared_type(declaredType), _type(type), _size(size), _length(length), _flatable(flatable), _binding(binding)
{
}

Uniform::Uniform(const String& name, const String& type, uint32_t length, const sp<Input>& flatable, int32_t binding)
    : Uniform(name, type, toType(type), getTypeSize(toType(type)), length, flatable, binding)
{
}

Uniform::Uniform(const String& name, Uniform::Type type, uint32_t length, const sp<Input>& flatable, int32_t binding)
    : Uniform(name, toDeclaredType(type), type, getTypeSize(type), length, flatable, binding)
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

uint32_t Uniform::length() const
{
    return _length;
}

size_t Uniform::size() const
{
    size_t s = _size * _length;
    DCHECK(!_flatable || _flatable->size() <= s, "Uniform buffer overflow, name: \"%s\", size: %d, flatable size: %d", _name.c_str(), s, _flatable->size());
    DWARN(!_flatable || _flatable->size() == s, "Uniform buffer size mismatch, name: \"%s\", size: %d, flatable size: %d", _name.c_str(), s, _flatable->size());
    return s;
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
    return TYPE_STRUCT;
}

String Uniform::toDeclaredType(Type type)
{
    switch(type) {
    case TYPE_I1V:
    case TYPE_I1:
        return "int";
    case TYPE_F1V:
    case TYPE_F1:
        return "float";
    case TYPE_F2V:
    case TYPE_F2:
        return "vec2";
    case TYPE_F3V:
    case TYPE_F3:
        return "vec3";
    case TYPE_F4V:
    case TYPE_F4:
        return "vec4";
    case TYPE_MAT4V:
    case TYPE_MAT4:
        return "mat4";
    case TYPE_MAT3V:
    case TYPE_MAT3:
        return "mat3";
    case TYPE_SAMPLER2D:
        return "sampler2D";
    default:
        break;
    }
    DFATAL("Unsupported type: %d", type);
    return "";
}

uint32_t Uniform::getTypeSize(Uniform::Type type)
{
    const size_t typeSizes[TYPE_COUNT] = {0, 4, 4, 8, 16, 16, 4, 4, 8, 16, 16, 36, 36, 64, 64, 4, 0};
    return typeSizes[type];
}

const String& Uniform::declaredType() const
{
    return _declared_type;
}

const sp<Input>& Uniform::flatable() const
{
    return _flatable;
}

void Uniform::setInput(const sp<Input>& flatable)
{
    _flatable = flatable;
}

String Uniform::declaration(const String& descriptor) const
{
    const String t = declaredType();
    if(_length == 1)
        return Strings::sprintf("%s%s %s;", descriptor.c_str(), t.c_str(), _name.c_str());
    return Strings::sprintf("%s%s %s[%d];", descriptor.c_str(), t.c_str(), _name.c_str(), _length);
}

}
