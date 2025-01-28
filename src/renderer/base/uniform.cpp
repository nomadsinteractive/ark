#include "renderer/base/uniform.h"

#include "core/base/observer.h"
#include "core/inf/array.h"
#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/util/boolean_type.h"
#include "core/util/strings.h"

namespace ark {

namespace {

String toDeclaredType(Uniform::Type type)
{
    switch(type) {
        case Uniform::TYPE_I1V:
        case Uniform::TYPE_I1:
            return "int";
        case Uniform::TYPE_F1V:
        case Uniform::TYPE_F1:
            return "float";
        case Uniform::TYPE_F2V:
        case Uniform::TYPE_F2:
            return "vec2";
        case Uniform::TYPE_F3V:
        case Uniform::TYPE_F3:
            return "vec3";
        case Uniform::TYPE_F4V:
        case Uniform::TYPE_F4:
            return "vec4";
        case Uniform::TYPE_MAT4V:
        case Uniform::TYPE_MAT4:
            return "mat4";
        case Uniform::TYPE_MAT3V:
        case Uniform::TYPE_MAT3:
            return "mat3";
        case Uniform::TYPE_SAMPLER2D:
            return "sampler2D";
        case Uniform::TYPE_IMAGE2D:
            return "image2D";
        case Uniform::TYPE_UIMAGE2D:
            return "uimage2D";
        case Uniform::TYPE_IIMAGE2D:
            return "iimage2D";
        case Uniform::TYPE_STRUCT:
            return "struct";
        default:
            break;
    }
    FATAL("Unsupported type: %d", type);
    return "";
}

}

Uniform::Uniform(String name, String declaredType, Uniform::Type type, uint32_t componentSize, uint32_t length, sp<Uploader> uploader)
    : _name(std::move(name)), _declared_type(std::move(declaredType)), _type(type), _component_size(componentSize), _length(length), _uploader(std::move(uploader))
{
}

Uniform::Uniform(String name, Type type, uint32_t componentSize, uint32_t length, sp<Uploader> uploader)
    : _name(std::move(name)), _declared_type(toDeclaredType(type)), _type(type), _component_size(componentSize), _length(length), _uploader(std::move(uploader))
{
}

Uniform::Uniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> uploader)
    : Uniform(std::move(name), toDeclaredType(type), type, getComponentSize(type), length, std::move(uploader))
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
    const size_t s = _component_size * _length;
    DCHECK(!_uploader || _uploader->size() <= s, "Uniform buffer overflow, name: \"%s\", size: %d, input size: %d", _name.c_str(), s, _uploader->size());
    DCHECK_WARN(!_uploader || _uploader->size() == s, "Uniform buffer size mismatch, name: \"%s\", size: %d, input size: %d", _name.c_str(), s, _uploader->size());
    return s;
}

static Optional<Uniform::Type> vecToUniformType(const String& declaredType, const String& vecPrefix, Uniform::Type baseType)
{
    if(declaredType.startsWith(vecPrefix) && declaredType.length() == vecPrefix.length() + 1)
    {
        const int32_t vs = declaredType.at(vecPrefix.length()) - '1';
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
    if(const Optional<Type> typeOpt = vecToUniformType(declaredType, "vec", TYPE_F1))
        return typeOpt.value();
    if(const Optional<Type> typeOpt = vecToUniformType(declaredType, "ivec", TYPE_I1))
        return typeOpt.value();
    if(declaredType.startsWith("v") && (declaredType.size() == 3 || declaredType.size() == 4))
    {
        const int32_t vs = declaredType.at(1) - '1';
        const char ts = declaredType.at(2);
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
    if(declaredType == "image2D")
        return TYPE_IMAGE2D;
    CHECK(declaredType == "struct", "Unknow type \"%s\"", declaredType.c_str());
    return TYPE_STRUCT;
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

void Uniform::setUploader(sp<Uploader> uploader)
{
    _uploader = std::move(uploader);
}

String Uniform::declaration(const String& descriptor) const
{
    const String t = declaredType();
    if(_length == 1)
        return Strings::sprintf("%s%s %s;", descriptor.c_str(), t.c_str(), _name.c_str());
    return Strings::sprintf("%s%s %s[%d];", descriptor.c_str(), t.c_str(), _name.c_str(), _length);
}

}
