#include "graphics/base/filter.h"

#include "core/base/bean_factory.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/impl/flatable/flatable_numeric.h"
#include "core/util/bean_utils.h"

#include "graphics/base/rect.h"

#include "renderer/base/gl_shader.h"

namespace ark {

namespace {

class AlphaNumericFlatableBuilder : public Builder<Flatable> {
public:
    AlphaNumericFlatableBuilder(const sp<Builder<Numeric>>& delegate)
        : _delegate(delegate) {
    }

    virtual sp<Flatable> build(const sp<Scope>& args) override {
        const sp<Numeric> alpha = _delegate->build(args);
        return sp<FlatableNumeric>::make(alpha);
    }

private:
    sp<Builder<Numeric>> _delegate;
};

}

Filter::Filter(const sp<Numeric>& alpha)
    : _alpha(Null::toSafe<Numeric>(alpha))
{
}

void Filter::addVarying(const Varying& varying)
{
    _varyings.push_back(varying);
}

void Filter::setVaryings(void* buf, uint32_t stride, uint32_t count) const
{
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    for(const Varying& i : _varyings)
        i.settle(ptr);

    for(uint32_t i = 0; i < count - 1; i++)
    {
        memcpy(ptr + stride, ptr, stride);
        ptr += stride;
    }
}

Filter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<GLShader>(manifest, Constants::Attributes::SHADER)),
      _alpha(factory.getBuilder<Numeric>(manifest, Constants::Attributes::ALPHA, false))
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        _varying_builders.push_back(VaryingBuilder(name, factory.ensureBuilderByTypeValue<Flatable>(type, value)));
    }
}

sp<Filter> Filter::BUILDER::build(const sp<Scope>& args)
{
    const sp<Filter> filter = sp<Filter>::make(_alpha ? _alpha->build(args) : nullptr);

    if(_shader)
    {
        initVaryings(args);
        _shader = nullptr;
    }

    for(const VaryingBuilder& i : _varying_builders)
        filter->addVarying(i.build(args));
    return filter;
}

void Filter::BUILDER::initVaryings(const sp<Scope>& args)
{
    const sp<GLShader> shader = _shader->build(args);
    if(shader)
    {
        if(_alpha)
            _varying_builders.push_back(VaryingBuilder("Alpha", sp<AlphaNumericFlatableBuilder>::make(_alpha)));
        std::set<String> names;
        for(VaryingBuilder& i : _varying_builders)
        {
            DCHECK(names.find(i._name) == names.end(), "Attribute name \"%s\" duplicated", i._name.c_str());
            names.insert(i._name);
            const GLAttribute& attr = shader->getAttribute(i._name);
            i._offset = attr.offset();
        }
    }
}

static sp<Filter> _create_null_filter()
{
    const sp<Filter> filter = sp<Filter>::make(nullptr);
    return filter;
}

template<> ARK_API const sp<Filter>& Null::ptr()
{
    static const sp<Filter> filter = _create_null_filter();
    return filter;
}

Filter::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _delegate(sp<Filter::BUILDER>::make(parent, Documents::fromProperties(value)))
{
}

sp<Filter> Filter::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate->build(args);
}

Filter::Varying::Varying(uint16_t offset, const sp<Flatable>& flatable)
    : _offset(offset), _flatable(flatable)
{
}

void Filter::Varying::settle(uint8_t* ptr) const
{
    _flatable->flat(ptr + _offset);
}

Filter::BUILDER::VaryingBuilder::VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable)
    : _name(name), _flatable(flatable), _offset(0)
{
}

Filter::BUILDER::VaryingBuilder::VaryingBuilder(const Filter::BUILDER::VaryingBuilder& other)
    : _name(other._name), _flatable(other._flatable), _offset(other._offset)
{
}

Filter::BUILDER::VaryingBuilder::VaryingBuilder(Filter::BUILDER::VaryingBuilder&& other)
    : _name(std::move(other._name)), _flatable(std::move(other._flatable)), _offset(other._offset)
{
}

Filter::Varying Filter::BUILDER::VaryingBuilder::build(const sp<Scope>& args) const
{
    return Varying(_offset, _flatable->build(args));
}

}
