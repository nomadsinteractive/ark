#ifndef ARK_GRAPHICS_IMPL_MOVIECLIP_LOOPING_MOVIECLIP_H_
#define ARK_GRAPHICS_IMPL_MOVIECLIP_LOOPING_MOVIECLIP_H_

#include <list>

#include "core/inf/builder.h"
#include "core/inf/iterator.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class LoopingMovieclip : public Movieclip {
public:
    LoopingMovieclip(const sp<Movieclip>& delegate, bool reverse);

    virtual bool hasNext() override;
    virtual sp<Renderer> next() override;

//  [[plugin::style("looping")]]
    class DECORATOR : public Builder<Movieclip> {
    public:
        DECORATOR(const sp<Builder<Movieclip>>& delegate, const String& value);

        virtual sp<Movieclip> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Movieclip>> _delegate;
        bool _reverse;
    };

private:
    sp<Renderer> begin();
    sp<Renderer> rbegin();

private:
    sp<Movieclip> _delegate;
    bool _reverse;

    std::list<sp<Renderer>> _frames;
    std::list<sp<Renderer>>::const_iterator _iterator;
    std::list<sp<Renderer>>::const_reverse_iterator _reverse_iterator;
    bool _reversing;

};

}

#endif
