#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_MATRIX_ARRAY_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_MATRIX_ARRAY_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableMatrixArray : public Flatable {
public:
    FlatableMatrixArray(const sp<Array<Matrix>>& matrixArray);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual uint32_t length() override;

//  [[plugin::builder::by-value("mat4v")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Flatable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Array<Matrix>>> _matrix_array;
    };

private:
    sp<Array<Matrix>> _matrix_array;

};

}

#endif
