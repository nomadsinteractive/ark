#ifndef ARK_CORE_IMPL_RANGE_LOOPS_RANGE_H_
#define ARK_CORE_IMPL_RANGE_LOOPS_RANGE_H_

#include <list>

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/iterator.h"
#include "core/types/shared_ptr.h"

namespace ark {

class LoopsRange : public Range {
public:
    LoopsRange(const sp<Range>& delegate, bool reverse);

    virtual bool hasNext() override;
    virtual int32_t next() override;

//  [[plugin::style("loops")]]
    class BUILDER : public Builder<Range> {
    public:
        BUILDER(BeanFactory& factory, const sp<Builder<Range>>& delegate, const String& style);

        virtual sp<Range> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Range>> _delegate;
        bool _reverse;
    };

private:
    int32_t begin();
    int32_t rbegin();

private:
    sp<Range> _delegate;
    bool _reverse;

    std::list<int32_t> _buffer;
    std::list<int32_t>::const_iterator _iterator;
    std::list<int32_t>::const_reverse_iterator _reverse_iterator;
    bool _reversing;

};

}

#endif
