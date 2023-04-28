#include "renderer/base/uniform.h"

#include "core/base/observer.h"
#include "core/base/notifier.h"
#include "core/inf/array.h"
#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/util/boolean_type.h"
#include "core/util/strings.h"

namespace ark {

Uniform::Uniform(String name, String declaredType, Uniform::Type type, size_t size, uint32_t length, sp<Uploader> input, int32_t binding)
    : _name(std::move(name)), _declared_type(std::move(declaredType)), _type(type), _component_size(size), _length(length), _uploader(std::move(input)), _binding(binding)
{
}

Uniform::Uniform(String name, String type, uint32_t length, sp<Uploader> input, int32_t binding)
    : Uniform(std::move(name), std::move(type), toType(type), getComponentSize(toType(type)), length, std::move(input), binding)
{
}

Uniform::Uniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> input, int32_t binding)
    : Uniform(std::move(name), toDeclaredType(type), type, getComponentSize(type), length, std::move(input), binding)
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
    DCHECK(!_uploader || _uploader->size() <= s, "Uniform buffer overflow, name: \"%s\", size: %d, input size: %d", _name.c_str(), s, _uploader->size());
    DCHECK_WARN(!_uploader || _uploader->size() == s, "Uniform buffer size mismatch, name: \"%s\", size: %d, input size: %d", _name.c_str(), s, _uploader->size());
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

static Optional<Uniform::Type> vecToUniformType(const String& declaredType, const String& vecPrefix, Uniform::Type baseType)
{
    if(declaredType.startsWith(vecPrefix) && declaredType.length() == vecPrefix.length() + 1)
    {
        int32_t vs = declaredType.at(vecPrefix.length()) - '1';
        CHECK(vs >= 0 && vs < 4, "Unknow type %s", declaredType.c_str());
        return static_cast<Uniform::Type>(baseType + vs);
    }
    return Optional<Uniform::Type>();
}

Uniform::Type Uniform::toType(const String& declaredType)
{
    if(declaredType == "int")
        return TYPE_I1;
    if(declaredType == "float")
        return TYPE_F1;
    Optional<Uniform::Type> typeOpt = vecToUniformType(declaredType, "vec", TYPE_F1);
    if(typeOpt)
        return typeOpt.value();
    typeOpt = vecToUniformType(declaredType, "ivec", TYPE_I1);
    if(typeOpt)
        return typeOpt.value();
    if(declaredType.startsWith("v") && (declaredType.size() == 3 || declaredType.size() == 4))
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
    if(declaredType == "uimage2D")
        return TYPE_UIMAGE2D;
    if(declaredType == "iimage2D")
        return TYPE_IIMAGE2D;
    CHECK(declaredType == "image2D", "Unknow type \"%s\"", declaredType.c_str());
    if(declaredType == "image2D")
        return TYPE_IMAGE2D;
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
    case TYPE_IMAGE2D:
        return "image2D";
    case TYPE_UIMAGE2D:
        return "uimage2D";
    case TYPE_IIMAGE2D:
        return "iimage2D";
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

const sp<Uploader>& Uniform::uploader() const
{
    return _uploader;
}

void Uniform::setUploader(const sp<Uploader>& uploader)
{
    _uploader = uploader;
}

String Uniform::declaration(const String& descriptor) const
{
    const String t = declaredType();
    if(_length == 1)
        return Strings::sprintf("%s%s %s;", descriptor.c_str(), t.c_str(), _name.c_str());
    return Strings::sprintf("%s%s %s[%d];", descriptor.c_str(), t.c_str(), _name.c_str(), _length);
}

}
