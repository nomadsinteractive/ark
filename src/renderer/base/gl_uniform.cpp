#include "renderer/base/gl_uniform.h"

#include "core/concurrent/dual.h"
#include "core/epi/changed.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/array/dynamic_array.h"
#include "core/util/strings.h"

#include "graphics/impl/flatable/flatable_color4f_array.h"

#include "renderer/base/gl_program.h"
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
        memcpy(buf, _buffer.front()->array(), _size);
    }

    virtual uint32_t size() override {
        return _size;
    }

    virtual uint32_t length() override {
        return _length;
    }

    virtual void run() override {
        if(!_changed || _changed->hasChanged()) {
            _delegate->flat(_buffer.back()->array());
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

GLUniform::GLUniform(const String& name, GLUniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed, const sp<RenderController>& renderController)
    : _name(name), _type(type), _flatable(flatable), _changed(changed)
{
    DWARN(renderController, "Initialize GLUniform \"%s\" without RenderController, which might cause multithreading concurrency problems", name.c_str());
    if(renderController)
        synchronize(renderController);
}

GLUniform::GLUniform(const GLUniform& other)
    : _name(other._name), _type(other._type), _flatable(other._flatable), _changed(other._changed)
{
}

const String& GLUniform::name() const
{
    return _name;
}

void GLUniform::prepare(GraphicsContext&, const sp<GLProgram>& program) const
{
    if(!_changed || _changed->hasChanged())
    {
        const GLProgram::Uniform& uniform = program->getUniform(_name);
        float buf[1024];
        DCHECK(_flatable->size() <= sizeof(buf), "Size too large: %d", _flatable->size());
        _flatable->flat(buf);
        switch(_type) {
        case UNIFORM_I1:
            DCHECK(_flatable->size() == 4, "Wrong uniform1i size: %d", _flatable->size());
            uniform.setUniform1i(*reinterpret_cast<int32_t*>(buf));
            break;
        case UNIFORM_F1:
            DCHECK(_flatable->size() == 4, "Wrong uniform1f size: %d", _flatable->size());
            uniform.setUniform1f(buf[0]);
            break;
        case UNIFORM_F2:
            DCHECK(_flatable->size() == 8, "Wrong uniform2f size: %d", _flatable->size());
            uniform.setUniform2f(buf[0], buf[1]);
            break;
        case UNIFORM_F3:
            DCHECK(_flatable->size() == 12, "Wrong uniform3f size: %d", _flatable->size());
            uniform.setUniform3f(buf[0], buf[1], buf[2]);
            break;
        case UNIFORM_F4:
            DCHECK(_flatable->size() == 16, "Wrong uniform4f size: %d", _flatable->size());
            uniform.setUniform4f(buf[0], buf[1], buf[2], buf[3]);
            break;
        case UNIFORM_F4V:
            DCHECK(_flatable->size() % 16 == 0, "Wrong uniform4fv size: %d", _flatable->size());
            uniform.setUniform4fv(_flatable->size() / 16, buf);
            break;
        case UNIFORM_MAT4V:
            DCHECK(_flatable->size() % 64 == 0, "Wrong color4fv size: %d", _flatable->size());
            uniform.setUniformMatrix4fv(_flatable->size() / 64, GL_FALSE, buf);
            break;
        default:
            DFATAL("Unimplemented");
        }
    }
}

String GLUniform::declaration() const
{
    String t;
    uint32_t s = 0;
    switch(_type) {
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

void GLUniform::synchronize(const sp<RenderController>& renderController)
{
    const sp<SynchronizedFlattable> synchronized = sp<SynchronizedFlattable>::make(_flatable, _changed);
    renderController->addPreUpdateRequest(synchronized);
    _flatable = synchronized;
    _changed = synchronized->notifier();
}

}
