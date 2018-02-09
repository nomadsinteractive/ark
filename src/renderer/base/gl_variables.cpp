#include "renderer/base/gl_variables.h"

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

GLVariables::GLVariables(const sp<Numeric>& alpha)
    : _alpha(Null::toSafe<Numeric>(alpha))
{
}

void GLVariables::addVarying(const Varying& varying)
{
    _varyings.push_back(varying);
}

void GLVariables::setVaryings(void* buf, uint32_t stride, uint32_t count) const
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

GLVariables::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
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

sp<GLVariables> GLVariables::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLVariables> filter = sp<GLVariables>::make(_alpha ? _alpha->build(args) : nullptr);

    if(_shader)
    {
        const sp<GLShader> shader = _shader->build(args);
        initVaryings(shader, args);
        _shader = nullptr;
    }

    for(const VaryingBuilder& i : _varying_builders)
        filter->addVarying(i.build(args));
    return filter;
}

void GLVariables::BUILDER::initVaryings(const sp<GLShader>& shader, const sp<Scope>& args)
{
    if(_alpha)
        _varying_builders.push_back(VaryingBuilder("Alpha", sp<AlphaNumericFlatableBuilder>::make(_alpha)));
    std::set<String> names;
    for(VaryingBuilder& i : _varying_builders)
    {
        DCHECK(names.find(i._name) == names.end(), "Duplicated attribute name \"%s\" ", i._name.c_str());
        names.insert(i._name);
        const GLAttribute& attr = shader->getAttribute(i._name);
        i._offset = attr.offset();
    }
}

static sp<GLVariables> _create_null_filter()
{
    const sp<GLVariables> filter = sp<GLVariables>::make(nullptr);
    return filter;
}

template<> ARK_API const sp<GLVariables>& Null::ptr()
{
    static const sp<GLVariables> filter = _create_null_filter();
    return filter;
}

GLVariables::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _delegate(sp<GLVariables::BUILDER>::make(parent, Documents::fromProperties(value)))
{
}

sp<GLVariables> GLVariables::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate->build(args);
}

GLVariables::Varying::Varying(uint16_t offset, const sp<Flatable>& flatable)
    : _offset(offset), _flatable(flatable)
{
}

void GLVariables::Varying::settle(uint8_t* ptr) const
{
    _flatable->flat(ptr + _offset);
}

GLVariables::BUILDER::VaryingBuilder::VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable)
    : _name(name), _flatable(flatable), _offset(0)
{
}

GLVariables::BUILDER::VaryingBuilder::VaryingBuilder(const GLVariables::BUILDER::VaryingBuilder& other)
    : _name(other._name), _flatable(other._flatable), _offset(other._offset)
{
}

GLVariables::BUILDER::VaryingBuilder::VaryingBuilder(GLVariables::BUILDER::VaryingBuilder&& other)
    : _name(std::move(other._name)), _flatable(std::move(other._flatable)), _offset(other._offset)
{
}

GLVariables::Varying GLVariables::BUILDER::VaryingBuilder::build(const sp<Scope>& args) const
{
    return Varying(_offset, _flatable->build(args));
}

}
