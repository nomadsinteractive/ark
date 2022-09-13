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
    : _name(name), _declared_type(declaredType), _type(type), _component_size(size), _length(length), _input(flatable), _binding(binding)
{
}

Uniform::Uniform(const String& name, const String& type, uint32_t length, const sp<Input>& flatable, int32_t binding)
    : Uniform(name, type, toType(type), getComponentSize(toType(type)), length, flatable, binding)
{
}

Uniform::Uniform(const String& name, Uniform::Type type, uint32_t length, const sp<Input>& flatable, int32_t binding)
    : Uniform(name, toDeclaredType(type), type, getComponentSize(type), length, flatable, binding)
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
    size_t s = _component_size * _length;
    DCHECK(!_input || _input->size() <= s, "Uniform buffer overflow, name: \"%s\", size: %d, input size: %d", _name.c_str(), s, _input->size());
    DWARN(!_input || _input->size() == s, "Uniform buffer size mismatch, name: \"%s\", size: %d, input size: %d", _name.c_str(), s, _input->size());
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
    if(declaredType.startsWith("vec") && declaredType.size() > 3)
    {
        int32_t vs = declaredType.at(3) - '1';
        CHECK(vs >= 0 && vs < 4, "Unknow type %s", declaredType.c_str());
        return static_cast<Type>(TYPE_F1 + vs);
    }
    if(declaredType.startsWith("v") && declaredType.size() > 2)
    {
        int32_t vs = declaredType.at(1) - '1';
        char ts = declaredType.at(2);
        CHECK(vs >= 0 && vs < 4 && (ts == 'i' || ts == 'f'), "Unknow type %s", declaredType.c_str());
        if(declaredType.endsWith("v"))
            return ts == 'f' ? static_cast<Type>(TYPE_F1V + vs) : static_cast<Type>(TYPE_I1V + vs);
        return ts == 'f' ? static_cast<Type>(TYPE_F1 + vs) : static_cast<Type>(TYPE_I1 + vs);
    }
    if(declaredType == "mat4")
        return TYPE_MAT4;
    if(declaredType == "mat4fv")
        return TYPE_MAT4V;
    if(declaredType == "mat3")
        return TYPE_MAT3;
    if(declaredType == "mat3fv")
        return TYPE_MAT3V;
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

uint32_t Uniform::getComponentSize(Uniform::Type type)
{
    switch (type) {
        case Uniform::TYPE_I1:
        case Uniform::TYPE_F1:
        case Uniform::TYPE_I1V:
        case Uniform::TYPE_F1V:
            return 4;
        case Uniform::TYPE_I2:
        case Uniform::TYPE_F2:
        case Uniform::TYPE_I2V:
        case Uniform::TYPE_F2V:
            return 8;
        case Uniform::TYPE_I3:
        case Uniform::TYPE_F3:
        case Uniform::TYPE_I3V:
        case Uniform::TYPE_F3V:
            return 12;
        case Uniform::TYPE_I4:
        case Uniform::TYPE_F4:
        case Uniform::TYPE_I4V:
        case Uniform::TYPE_F4V:
            return 16;
        case Uniform::TYPE_MAT3:
        case Uniform::TYPE_MAT3V:
            return 36;
        case Uniform::TYPE_MAT4:
        case Uniform::TYPE_MAT4V:
            return 64;
        default:
            break;
    }
    return 0;
}

const String& Uniform::declaredType() const
{
    return _declared_type;
}

const sp<Input>& Uniform::input() const
{
    return _input;
}

void Uniform::setInput(const sp<Input>& flatable)
{
    _input = flatable;
}

String Uniform::declaration(const String& descriptor) const
{
    const String t = declaredType();
    if(_length == 1)
        return Strings::sprintf("%s%s %s;", descriptor.c_str(), t.c_str(), _name.c_str());
    return Strings::sprintf("%s%s %s[%d];", descriptor.c_str(), t.c_str(), _name.c_str(), _length);
}

}
