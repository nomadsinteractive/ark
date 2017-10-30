#include "core/impl/range/loops_range.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"

namespace ark {

LoopsRange::LoopsRange(const sp<Range>& delegate, bool reverse)
    : _delegate(delegate), _reverse(reverse)
{
}

bool LoopsRange::hasNext()
{
    return true;
}

int32_t LoopsRange::next()
{
    if(_delegate)
    {
        if(_delegate->hasNext())
        {
            int32_t next = _delegate->next();
            _buffer.push_back(next);
            return next;
        }
        DCHECK(_buffer.size() > 0, "Empty int32_ts");
        _delegate = nullptr;
        return _reverse ? rbegin() : begin();
    }
    if(_reversing)
    {
        if(_reverse_iterator != _buffer.rend())
        {
            int32_t next = *_reverse_iterator;
            ++_reverse_iterator;
            return next;
        }
        return begin();
    }
    if(_iterator != _buffer.end())
    {
        int32_t next = *_iterator;
        ++_iterator;
        return next;
    }
    return _reverse ? rbegin() : begin();
}

int32_t LoopsRange::begin()
{
    _reversing = false;
    _iterator = _buffer.begin();
    int32_t next = *_iterator;
    ++_iterator;
    return next;
}

int32_t LoopsRange::rbegin()
{
    _reversing = true;
    _reverse_iterator = _buffer.rbegin();
    int32_t next = *_reverse_iterator;
    ++_reverse_iterator;
    return next;
}

LoopsRange::BUILDER::BUILDER(BeanFactory& /*factory*/, const sp<Builder<Range>>& delegate, const String& style)
    : _delegate(delegate), _reverse(style == "reverse")
{
    DCHECK(style == "reverse" || style == "restart", "Style loops should be either \"reverse\" or \"restart\", but \"%s\" given", style.c_str());
}

sp<Range> LoopsRange::BUILDER::build(const sp<Scope>& args)
{
    return sp<Range>::adopt(new LoopsRange(_delegate->build(args), _reverse));
}

}
