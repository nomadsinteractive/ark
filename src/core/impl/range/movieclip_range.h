#ifndef ARK_CORE_IMPL_RANGE_MOVIECLIP_RANGE_H_
#define ARK_CORE_IMPL_RANGE_MOVIECLIP_RANGE_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/iterator.h"
#include "core/types/shared_ptr.h"

namespace ark {

class MovieclipRange : public Range {
public:
    MovieclipRange(const sp<Range>& delegate, const sp<Numeric>& duration, float interval);

    virtual bool hasNext() override;
    virtual int32_t next() override;

//  [[plugin::builder("movieclip")]]
    class BUILDER : public Builder<Range> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Range> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Range>> _delegate;
        sp<Builder<Duration>> _duration;
        float _interval;
    };

private:
    sp<Range> _delegate;
    sp<Numeric> _duration;
    float _interval;
    float _next_update;
    int32_t _last_value;
    bool _has_next1;
    bool _has_next2;

};

}

#endif
