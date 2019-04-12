#include "renderer/base/varyings.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/memory_pool.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/impl/flatable/flatable_by_variable.h"
#include "core/util/bean_utils.h"

#include "graphics/base/rect.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/varyings.h"

namespace ark {

Varyings::Varyings(const Shader& shader)
    : _pipeline_input(shader.input()), _size(0)
{
}

Varyings::Varyings()
    : _size(0)
{
}

void Varyings::flat(void* buf)
{
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    for(const auto& i : _varyings)
        i.second.apply(ptr);
}

uint32_t Varyings::size()
{
    return _size;
}

void Varyings::addVarying(const String& name, const sp<Flatable>& flatable)
{
    DCHECK(_varyings.find(name) == _varyings.end(), "Varying \"%s\" already exists", name.c_str());
    int32_t offset = _pipeline_input->getAttributeOffset(name);
    DCHECK(offset >= 0, "Illegal Varying, name: \"%s\", offset: %d", name.c_str(), offset);
    _varyings[name] = Varying(offset, flatable);
    _size = std::max<uint32_t>(offset + flatable->size(), _size);
}

void Varyings::add(const String& name, const sp<Numeric>& var)
{
    addVarying(name, sp<FlatableByVariable<float>>::make(var));
}

Varyings::Snapshot Varyings::snapshot(MemoryPool& memoryPool) const
{
    if(!_size)
        return Snapshot();

    const bytearray bytes = memoryPool.allocate(_size);
    uint8_t* ptr = reinterpret_cast<uint8_t*>(bytes->buf());
    for(const auto& i : _varyings)
        i.second.apply(ptr);
    return bytes;
}

Varyings::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    for(const document& i : manifest->children("varying"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        _varying_builders.emplace_back(name, factory.ensureBuilderByTypeValue<Flatable>(type, value));
    }

    if(_varying_builders.size() > 0)
        _shader = factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER);
}

sp<Varyings> Varyings::BUILDER::build(const sp<Scope>& args)
{
    if(!_shader)
        return nullptr;

    const sp<Shader> shader = _shader->build(args);
    const sp<Varyings> varyings = sp<Varyings>::make(shader);

    for(const VaryingBuilder& i : _varying_builders)
        varyings->addVarying(i._name,  i._flatable->build(args));
    return varyings;
}

template<> ARK_API sp<Varyings> Null::ptr()
{
    return Ark::instance().obtain<Varyings>();
}

Varyings::Varying::Varying(int32_t offset, const sp<Flatable>& flatable)
    : _offset(offset), _flatable(flatable)
{
    DCHECK(offset >= 0, "Invail varying offset: %d", offset);
}

Varyings::Varying::Varying()
    : _offset(0)
{
}

void Varyings::Varying::apply(uint8_t* ptr) const
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

}
