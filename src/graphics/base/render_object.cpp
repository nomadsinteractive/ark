#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/base/variable_wrapper.h"
#include "core/inf/iterator.h"
#include "core/inf/variable.h"
#include "core/epi/expired.h"
#include "core/util/bean_utils.h"

#include "graphics/base/size.h"
#include "renderer/base/varyings.h"
#include "graphics/base/vec2.h"

namespace ark {

namespace {

class IntegerRange : public Integer {
public:
    IntegerRange(const sp<Range>& range, const sp<Expired>& expired)
        : _range(range), _expired(expired) {
    }

    virtual int32_t val() override {
        bool hasNext = _range->hasNext();
        if(hasNext)
            return _range->next();
        if(_expired)
            _expired->expire();
        return 0;
    }

private:
    sp<Range> _range;
    sp<Expired> _expired;
};

}

RenderObject::RenderObject(int32_t type, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& filter)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(Null::toSafe<Varyings>(filter))
{
}

RenderObject::RenderObject(const sp<Integer>& type, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& filter)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(Null::toSafe<Varyings>(filter))
{
}

const sp<Integer> RenderObject::type() const
{
    return _type;
}

const sp<Varyings>& RenderObject::filter() const
{
    return _varyings;
}

float RenderObject::width() const
{
    return _size->width();
}

float RenderObject::height() const
{
    return _size->height();
}

const sp<Size>& RenderObject::size()
{
    return _size.ensure();
}

const sp<Transform>& RenderObject::transform() const
{
    return _transform.ensure();
}

void RenderObject::setType(int32_t type)
{
    _type->set(type);
}

float RenderObject::x() const
{
    return _position->val().x();
}

float RenderObject::y() const
{
    return _position->val().y();
}

V2 RenderObject::xy() const
{
    return _position->val();
}

const sp<VV>& RenderObject::position() const
{
    return _position.ensure();
}

void RenderObject::setPosition(const sp<VV>& position)
{
    _position.assign(position);
}

void RenderObject::setSize(const sp<Size>& size)
{
    _size.assign(size);
}

void RenderObject::setTransform(const sp<Transform>& transform)
{
    _transform.assign(transform);
}

void RenderObject::setFilter(const sp<Varyings>& filter)
{
    _varyings = Null::toSafe<Varyings>(filter);
}

void RenderObject::setTag(const Box& tag)
{
    _tag = tag;
}

const Box& RenderObject::tag() const
{
    return _tag;
}

RenderObject::Snapshot RenderObject::snapshot(MemoryPool& memoryPool) const
{
    return Snapshot(_type->val(), _position->val(), V(_size->width(), _size->height()), _transform->snapshot(), _varyings->snapshot(memoryPool));
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& doc)
    : _type(factory.ensureBuilder<Integer>(doc, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec>(doc, Constants::Attributes::POSITION)),
      _size(factory.getBuilder<Size>(doc, Constants::Attributes::SIZE)),
      _transform(factory.getBuilder<Transform>(doc, Constants::Attributes::TRANSFORM)),
      _varyings(factory.getBuilder<Varyings>(doc, Constants::Attributes::VARYINGS))
{
}

sp<RenderObject> RenderObject::BUILDER::build(const sp<Scope>& args)
{
    const sp<Integer> type = _type->build(args);
    return sp<RenderObject>::make(type, _position->build(args), _size->build(args), _transform->build(args), _varyings->build(args));
}

RenderObject::EXPIRED_STYLE::EXPIRED_STYLE(BeanFactory& parent, const sp<Builder<RenderObject>>& delegate, const String& value)
    : _delegate(delegate), _expired(parent.ensureBuilder<Expired>(value))
{
}

sp<RenderObject> RenderObject::EXPIRED_STYLE::build(const sp<Scope>& args)
{
    return _delegate->build(args).absorb(_expired->build(args));
}

RenderObject::Snapshot::Snapshot(uint32_t type, const V& position, const V& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
    : _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

}
