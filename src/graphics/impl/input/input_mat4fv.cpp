#include "graphics/impl/input/input_mat4fv.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class InputArray : public Input {
public:
    InputArray(sp<Array<sp<Input>>> inputs)
        : _inputs(std::move(inputs)), _size(0) {
        for(const sp<Input>& i : * _inputs) {
            uint32_t s = i->size();
            DASSERT((s % 64) == 0);
            _size += s;
        }
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const sp<Input>& i : * _inputs)
            dirty = i->update(timestamp) || dirty;
        return dirty;
    }

    virtual void flat(void* buf) override {
        uint32_t offset = 0;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
        for(const sp<Input>& i : * _inputs) {
            uint32_t s = i->size();
            i->flat(ptr + offset);
            offset += s;
        }
    }

    virtual uint32_t size() override {
        return _size;
    }

private:
    sp<Array<sp<Input>>> _inputs;
    uint32_t _size;
};

}

InputMat4fv::InputMat4fv(array<sp<Mat4>> array)
    : _array(std::move(array))
{
}

void InputMat4fv::flat(void* buf)
{
    M4* vbuf = reinterpret_cast<M4*>(buf);
    uint32_t idx = 0;
    for(const sp<Mat4>& i : *_array)
        vbuf[idx++] = i->val();
}

uint32_t InputMat4fv::size()
{
    return _array->length() * sizeof(M4);
}

bool InputMat4fv::update(uint64_t timestamp)
{
    bool dirty = false;
    for(const sp<Mat4>& i : *_array)
        dirty = i->update(timestamp) || dirty;
    return dirty;
}

InputMat4fv::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _id(Identifier::parse(value)), _array(factory.getBuilder<Array<sp<Mat4>>>(value))
{
}

sp<Input> InputMat4fv::BUILDER::build(const Scope& args)
{
    if(_id.isArg())
    {
        array<sp<Mat4>> mats = args.build<Array<sp<Mat4>>>(_id.arg(), args);
        if(mats)
            return sp<InputMat4fv>::make(std::move(mats));

        sp<Input> flatable = args.build<Input>(_id.arg(), args);
        if(flatable)
            return flatable;

        array<sp<Input>> flatables = args.build<Array<sp<Input>>>(_id.arg(), args);
        if(flatables)
            return sp<InputArray>::make(std::move(flatables));
    }

    return sp<InputMat4fv>::make(_array->build(args));
}

}
