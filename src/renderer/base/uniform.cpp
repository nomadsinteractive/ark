#include "renderer/base/uniform.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/util/strings.h"

namespace ark {

Uniform::Uniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Notifier>& notifier, int32_t binding)
    : _name(name), _type(type), _flatable(flatable), _notifier(notifier), _observer(_notifier ? _notifier->createObserver() : sp<Observer>::null()), _binding(binding)
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

String Uniform::getDeclaredType() const
{
    switch(_type) {
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
    DFATAL("Unsupported type: %d", _type);
    return "";
}

const sp<Flatable>& Uniform::flatable() const
{
    return _flatable;
}

void Uniform::setFlatable(const sp<Flatable>& flatable)
{
    _flatable = flatable;
}

void Uniform::setNotifier(const sp<Notifier>& notifier)
{
    DASSERT(notifier);
    _notifier = notifier;
    _observer = _notifier->createObserver();
}

bool Uniform::dirty() const
{
    return _observer ? _observer->val() : true;
}

String Uniform::declaration(const String& descriptor) const
{
    const String t = getDeclaredType();
    return Strings::sprintf("%s%s %s;", descriptor.c_str(), t.c_str(), _name.c_str());
}

void Uniform::notify() const
{
    if(_notifier)
        _notifier->notify();
}

}
