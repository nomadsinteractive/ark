#include "renderer/base/uniform.h"

#include "core/concurrent/dual.h"
#include "core/epi/changed.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/array/dynamic_array.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/strings.h"

#include "graphics/impl/flatable/flatable_color4f_array.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

namespace {

class SynchronizedFlattable : public Flatable, public Runnable {
public:
    SynchronizedFlattable(const sp<Flatable>& delegate, const sp<Changed>& changed)
        : _delegate(delegate), _changed(changed), _notifier(sp<Changed>::make(false)), _size(delegate->size()), _length(delegate->length()),
          _buffer(sp<DynamicArray<uint8_t>>::make(_size), sp<DynamicArray<uint8_t>>::make(_size)) {
        run();
    }

    virtual void flat(void* buf) override {
        memcpy(buf, _buffer.front()->buf(), _size);
    }

    virtual uint32_t size() override {
        return _size;
    }

    virtual uint32_t length() override {
        return _length;
    }

    virtual void run() override {
        if(!_changed || _changed->hasChanged()) {
            _delegate->flat(_buffer.back()->buf());
            _buffer.swap();
            _notifier->change();
        }
    }

    const sp<Changed>& notifier() const {
        return _notifier;
    }

private:
    sp<Flatable> _delegate;
    sp<Changed> _changed;
    sp<Changed> _notifier;
    uint32_t _size;
    uint32_t _length;

    Dual<array<uint8_t>> _buffer;
};

}

Uniform::Uniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed, const sp<RenderController>& renderController)
    : _name(name), _type(type), _flatable(flatable), _notifier(changed)
{
    DWARN(renderController, "Initialize GLUniform \"%s\" without RenderController, which might cause multithreading concurrency problems", name.c_str());
    if(renderController)
        synchronize(renderController);
}

Uniform::Uniform(const Uniform& other)
    : _name(other._name), _type(other._type), _flatable(other._flatable), _notifier(other._notifier)
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
    case UNIFORM_I1V:
        s = _flatable->size() / 4;
    case UNIFORM_I1:
        t = "int";
        break;
    case UNIFORM_F1V:
        s = _flatable->size() / 4;
    case UNIFORM_F1:
        t = "float";
        break;
    case UNIFORM_F2V:
        s = _flatable->size() / 8;
    case UNIFORM_F2:
        t = "vec2";
        break;
    case UNIFORM_F3V:
        s = _flatable->size() / 12;
    case UNIFORM_F3:
        t = "vec3";
        break;
    case UNIFORM_F4V:
        s = _flatable->size() / 16;
    case UNIFORM_F4:
        t = "vec4";
        break;
    case UNIFORM_MAT4V:
        s = _flatable->size() / 64;
    case UNIFORM_MAT4:
        t = "mat4";
    }
    return s ? Strings::sprintf("uniform %s %s[%d];", t.c_str(), _name.c_str(), s + 1) : Strings::sprintf("uniform %s %s;", t.c_str(), _name.c_str());
}

void Uniform::notify() const
{
    _notifier->change();
}

void Uniform::synchronize(const sp<RenderController>& renderController)
{
    const sp<SynchronizedFlattable> synchronized = sp<SynchronizedFlattable>::make(_flatable, _notifier);
    renderController->addPreUpdateRequest(synchronized, sp<BooleanByWeakRef<Flatable>>::make(synchronized, 1));
    _flatable = synchronized;
    _notifier = synchronized->notifier();
}

}
