#ifndef ARK_CORE_IMPL_RANGE_INTEGER_ARRAY_H_
#define ARK_CORE_IMPL_RANGE_INTEGER_ARRAY_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/inf/iterator.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[core::class]]
class ARK_API IntegerArray : public Array<int32_t>, public Range {
public:
    IntegerArray(const sp<Array<int32_t>>& array);

    virtual uint32_t length() override;
    virtual int32_t* array() override;

    virtual bool hasNext() override;
    virtual int32_t next() override;

//  [[plugin::builder::by-value]]
    class DICTIONARY_IMPL1 : public Builder<Array<int32_t>> {
    public:
        DICTIONARY_IMPL1(const String& value);

        virtual sp<Array<int32_t>> build(const sp<Scope>& args) override;
        sp<IntegerArray> buildIntegerArray(const sp<Scope>& args) const;

    private:
        sp<Array<int32_t>> _array;
    };


//  [[plugin::builder::by-value]]
    class DICTIONARY_IMPL2 : public Builder<Range> {
    public:
        DICTIONARY_IMPL2(const String& value);

        virtual sp<Range> build(const sp<Scope>& args) override;

    private:
        DICTIONARY_IMPL1 _impl;
    };

private:
    sp<Array<int32_t>> _array;
    uint32_t _iter;
};

}

#endif
