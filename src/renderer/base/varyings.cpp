#include "renderer/base/varyings.h"

#include "core/ark.h"
#include "core/base/allocator.h"
#include "core/base/bean_factory.h"
#include "core/inf/input.h"
#include "core/inf/variable.h"
#include "core/impl/flatable/flatable_by_variable.h"
#include "core/util/holder_util.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/base/v4.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/varyings.h"

namespace ark {

Varyings::Varyings()
    : _size(0)
{
}

void Varyings::traverse(const Holder::Visitor& visitor)
{
    for(const auto& iter : _varyings)
        HolderUtil::visit(iter.second._flatable, visitor);
}

bool Varyings::update(uint64_t timestamp) const
{
    bool dirty = false;
    for(const auto& i : _varyings)
        if(i.second._flatable->update(timestamp))
            dirty = true;
    return dirty;
}

Box Varyings::getProperty(const String& name) const
{
    const auto iter = _properties.find(name);
    CHECK(iter != _properties.end(), "Varyings has no property \"%s\"", name.c_str());
    return iter->second;
}

void Varyings::setVarying(const String& name, sp<Input> flatable)
{
    auto iter = _varyings.find(name);
    if(iter == _varyings.end())
    {
        _varyings.emplace(name, std::move(flatable));
        _size = 0;
    }
    else
    {
        DCHECK(iter->second._flatable->size() == flatable->size(), "Replacing existing varying \"%s\"(%d) with a different size value(%d)", name.c_str(), iter->second._flatable->size(), flatable->size());
        iter->second = Varying(std::move(flatable), iter->second._offset);
    }
}

void Varyings::setProperty(const String& name, const sp<Numeric>& var)
{
    _properties[name] = var;
    setVarying(name, sp<Input>::make<FlatableByVariable<float>>(var));
}

void Varyings::setProperty(const String& name, const sp<Vec2>& var)
{
    _properties[name] = var;
    setVarying(name, sp<Input>::make<FlatableByVariable<V2>>(var));
}

void Varyings::setProperty(const String& name, const sp<Vec3>& var)
{
    _properties[name] = var;
    setVarying(name, sp<Input>::make<FlatableByVariable<V3>>(var));
}

void Varyings::setProperty(const String& name, const sp<Vec4>& var)
{
    _properties[name] = var;
    setVarying(name, sp<Input>::make<FlatableByVariable<V4>>(var));
}

Varyings::Snapshot Varyings::snapshot(const PipelineInput& pipelineInput, Allocator& allocator)
{
    if(!_varyings.size())
        return Snapshot();

    if(!_size)
    {
        const PipelineInput::Stream& varyingStream = pipelineInput.streams().rbegin()->second;
        for(auto& i : _varyings)
        {
            i.second._offset = varyingStream.getAttributeOffset(i.first);
            DCHECK(i.second._offset >= 0, "Varying has no attribute \"%s\", offset: %d. Did you mean \"%s\"?", i.first.c_str(), i.second._offset,
                   Math::levensteinNearest(i.first, varyingStream.attributes().keys()).first.c_str());
            _size = std::max<uint32_t>(static_cast<uint32_t>(i.second._offset) + i.second._flatable->size(), _size);
        }
    }

    ByteArray::Borrowed memory = allocator.sbrk(_size);
    for(const auto& i : _varyings)
        i.second.apply(memory.buf());
    return Snapshot(memory);
}

Varyings::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    for(const document& i : manifest->children("varying"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        _varying_builders.emplace_back(name, factory.ensureBuilderByTypeValue<Input>(type, value));
    }
}

sp<Varyings> Varyings::BUILDER::build(const Scope& args)
{
    if(_varying_builders.size() == 0)
        return sp<Varyings>::make();

    const sp<Varyings> varyings = sp<Varyings>::make();
    for(const VaryingBuilder& i : _varying_builders)
        varyings->setVarying(i._name,  i._flatable->build(args));
    return varyings;
}

template<> ARK_API sp<Varyings> Null::ptr()
{
    return sp<Varyings>::make();
}

Varyings::Varying::Varying(const sp<Input>& flatable, int32_t offset)
    : _flatable(flatable), _offset(offset)
{
}

Varyings::Varying::Varying()
    : _offset(-1)
{
}

void Varyings::Varying::apply(uint8_t* ptr) const
{
    DASSERT(_offset >= 0);
    _flatable->flat(ptr + _offset);
}

Varyings::BUILDER::VaryingBuilder::VaryingBuilder(const String& name, const sp<Builder<Input>>& flatable)
    : _name(name), _flatable(flatable)
{
}

Varyings::Snapshot::Snapshot()
    : _memory(nullptr, 0)
{
}

Varyings::Snapshot::Snapshot(ByteArray::Borrowed memory)
    : _memory(memory)
{
}

Varyings::Snapshot::operator bool() const
{
    return _memory.buf() != nullptr;
}

}
