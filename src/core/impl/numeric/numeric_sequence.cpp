#include "core/impl/numeric/numeric_sequence.h"

#include "core/dom/document.h"
#include "core/base/bean_factory.h"
#include "core/util/math.h"

namespace ark {

NumericSequence::NumericSequence(const List<document>& sequence, bool looping, BeanFactory& beanFactory, const sp<Scope>& args)
    : _sequence(sequence), _iterator(_sequence.begin()), _bean_factory(beanFactory), _args(args), _looping(looping), _next_value(0), _previous_value(0)
{
    DCHECK(_iterator != _sequence.end(), "Empty numeric sequence");
    moveToNext();
}

float NumericSequence::val()
{
    float value = _current->val();
    if(Math::between<float>(_previous_value, value, _next_value))
        moveToNext();
    _previous_value = value;
    return value;
}

void NumericSequence::moveToNext()
{
    _current = _bean_factory.ensure<Numeric>(*_iterator, _args);
    ++_iterator;
    if(_iterator == _sequence.end())
    {
        if(!_looping)
            return;
        _iterator = _sequence.begin();
    }
    _next_value = _bean_factory.ensure<Numeric>(*_iterator, "s", _args)->val();
    _previous_value = _current->val();
}

NumericSequence::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _bean_factory(parent), _manifest(manifest), _looping(Documents::getAttribute<bool>(manifest, "looping", false))
{
}

sp<Numeric> NumericSequence::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new NumericSequence(_manifest->children(), _looping, _bean_factory, args));
}

}
