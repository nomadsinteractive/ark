#include "renderer/base/varyings.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/memory_pool.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/impl/flatable/flatable_numeric.h"
#include "core/util/bean_utils.h"

#include "graphics/base/rect.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_source.h"
#include "renderer/base/varyings.h"

namespace ark {

Varyings::Varyings(const GLShader& shader)
    : _shader_source(shader.source()), _size(0)
{
}

Varyings::Varyings()
    : _size(0)
{
}

void Varyings::addVarying(const String& name, const sp<Flatable>& flatable)
{
    DCHECK(_varyings.find(name) == _varyings.end(), "Shader variable \"%s\" already exists", name.c_str());
    uint32_t offset = _shader_source->getAttribute(name).offset();
    _varyings[name] = Varying(offset, flatable);
    _size = std::max<size_t>(offset + flatable->size(), _size);
}

void Varyings::add(const String& name, const sp<Numeric>& var)
{
    addVarying(name, sp<FlatableNumeric>::make(var));
}

Varyings::Snapshot Varyings::snapshot(MemoryPool& memoryPool) const
{
    if(!_size)
        return Snapshot(nullptr);

    const bytearray bytes = memoryPool.allocate(_size);
    uint8_t* ptr = reinterpret_cast<uint8_t*>(bytes->array());
    for(const auto iter : _varyings)
        iter.second.settle(ptr);
    return bytes;
}

Varyings::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<GLShader>(manifest, Constants::Attributes::SHADER))
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        _varying_builders.push_back(VaryingBuilder(name, factory.ensureBuilderByTypeValue<Flatable>(type, value)));
    }
}

sp<Varyings> Varyings::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLShader> shader = _shader->build(args);
    const sp<Varyings> varyings = sp<Varyings>::make(shader);

    for(const VaryingBuilder& i : _varying_builders)
        varyings->addVarying(i._name,  i._flatable->build(args));
    return varyings;
}

template<> ARK_API const sp<Varyings> Null::ptr()
{
    return Ark::instance().obtain<Varyings>();
}

Varyings::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _delegate(sp<Varyings::BUILDER>::make(parent, Documents::fromProperties(value)))
{
}

sp<Varyings> Varyings::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate->build(args);
}

Varyings::Varying::Varying(uint16_t offset, const sp<Flatable>& flatable)
    : _offset(offset), _flatable(flatable)
{
}

Varyings::Varying::Varying()
    : _offset(0)
{
}

void Varyings::Varying::settle(uint8_t* ptr) const
{
    _flatable->flat(ptr + _offset);
}

Varyings::BUILDER::VaryingBuilder::VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable)
    : _name(name), _flatable(flatable)
{
}

Varyings::Snapshot::Snapshot(const bytearray& bytes)
    : _bytes(bytes)
{
}

void Varyings::Snapshot::apply(void* buf, uint32_t stride, uint32_t count) const
{
    if(!_bytes)
        return;

    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    for(uint32_t i = 0; i < count; i++)
    {
        memcpy(ptr, _bytes->array(), _bytes->length());
        ptr += stride;
    }
}

}
