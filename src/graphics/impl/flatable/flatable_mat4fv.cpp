#include "graphics/impl/flatable/flatable_mat4fv.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class FlatableMat4fvFlatableArray : public Flatable {
public:
    FlatableMat4fvFlatableArray(sp<Array<sp<Flatable>>> flatables)
        : _flatables(std::move(flatables)), _size(0) {
        for(const sp<Flatable>& i : * _flatables) {
            uint32_t s = i->size();
            DASSERT((s % 64) == 0);
            _size += s;
        }
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const sp<Flatable>& i : * _flatables)
            dirty = i->update(timestamp) || dirty;
        return dirty;
    }

    virtual void flat(void* buf) override {
        uint32_t offset = 0;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
        for(const sp<Flatable>& i : * _flatables) {
            uint32_t s = i->size();
            i->flat(ptr + offset);
            offset += s;
        }
    }

    virtual uint32_t size() override {
        return _size;
    }

private:
    sp<Array<sp<Flatable>>> _flatables;
    uint32_t _size;
};

}

FlatableMat4fv::FlatableMat4fv(array<sp<Mat4>> array)
    : _array(std::move(array))
{
}

void FlatableMat4fv::flat(void* buf)
{
    M4* vbuf = reinterpret_cast<M4*>(buf);
    uint32_t idx = 0;
    for(const sp<Mat4>& i : *_array)
        vbuf[idx++] = i->val();
}

uint32_t FlatableMat4fv::size()
{
    return _array->length() * sizeof(M4);
}

bool FlatableMat4fv::update(uint64_t timestamp)
{
    bool dirty = false;
    for(const sp<Mat4>& i : *_array)
        dirty = i->update(timestamp) || dirty;
    return dirty;
}

FlatableMat4fv::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _id(Identifier::parse(value)), _array(factory.getBuilder<Array<sp<Mat4>>>(value))
{
}

sp<Flatable> FlatableMat4fv::BUILDER::build(const Scope& args)
{
    if(_id.isArg())
    {
        array<sp<Mat4>> mats = args.get<Array<sp<Mat4>>>(_id.arg());
        if(mats)
            return sp<FlatableMat4fv>::make(std::move(mats));

        sp<Flatable> flatable = args.get<Flatable>(_id.arg());
        if(flatable)
            return flatable;

        array<sp<Flatable>> flatables = args.get<Array<sp<Flatable>>>(_id.arg());
        if(flatables)
            return sp<FlatableMat4fvFlatableArray>::make(std::move(flatables));
    }

    return sp<FlatableMat4fv>::make(_array->build(args));
}

}
