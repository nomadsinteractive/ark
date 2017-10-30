#include "graphics/impl/movieclip/looping_movieclip.h"

#include "core/base/string.h"

namespace ark {

LoopingMovieclip::LoopingMovieclip(const sp<Movieclip>& delegate, bool reverse)
    : _delegate(delegate), _reverse(reverse)
{
}

bool LoopingMovieclip::hasNext()
{
    return true;
}

sp<Renderer> LoopingMovieclip::next()
{
    if(_delegate)
    {
        sp<Renderer> next = _delegate->next();
        if(next)
        {
            _frames.push_back(next);
            return next;
        }
        DCHECK(_frames.size() > 0, "Empty sp<Renderer>s");
        _delegate = nullptr;
        return _reverse ? rbegin() : begin();
    }
    if(_reversing)
    {
        if(_reverse_iterator != _frames.rend())
        {
            sp<Renderer> next = *_reverse_iterator;
            ++_reverse_iterator;
            return next;
        }
        return begin();
    }
    if(_iterator != _frames.end())
    {
        sp<Renderer> next = *_iterator;
        ++_iterator;
        return next;
    }
    return _reverse ? rbegin() : begin();
}

sp<Renderer> LoopingMovieclip::begin()
{
    _reversing = false;
    _iterator = _frames.begin();
    sp<Renderer> next = *_iterator;
    ++_iterator;
    return next;
}

sp<Renderer> LoopingMovieclip::rbegin()
{
    _reversing = true;
    _reverse_iterator = _frames.rbegin();
    sp<Renderer> next = *_reverse_iterator;
    ++_reverse_iterator;
    return next;
}

LoopingMovieclip::DECORATOR::DECORATOR(const sp<Builder<Movieclip>>& delegate, const String& value)
    : _delegate(delegate), _reverse(value == "reverse") {
}

sp<Movieclip> LoopingMovieclip::DECORATOR::build(const sp<Scope>& args)
{
    return sp<LoopingMovieclip>::make(_delegate->build(args), _reverse);
}

}
