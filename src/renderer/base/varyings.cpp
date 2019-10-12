#include "renderer/base/varyings.h"

#include "core/ark.h"
#include "core/base/allocator.h"
#include "core/base/bean_factory.h"
#include "core/base/memory_pool.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/impl/flatable/flatable_by_variable.h"
#include "core/util/holder_util.h"

#include "graphics/base/rect.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_layout.h"
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

void Varyings::addVarying(const String& name, const sp<Flatable>& flatable)
{
    DCHECK(_varyings.find(name) == _varyings.end(), "Varying \"%s\" already exists", name.c_str());
    _varyings.emplace(name, flatable);
    _size = 0;
}

void Varyings::add(const String& name, const sp<Numeric>& var)
{
    addVarying(name, sp<FlatableByVariable<float>>::make(var));
}

Varyings::Snapshot Varyings::snapshot(const PipelineInput& pipelineInput, Allocator& allocator)
{
    if(!_varyings.size())
        return Snapshot();

    if(!_size)
        for(auto& i : _varyings)
        {
            i.second._offset = pipelineInput.getAttributeOffset(i.first);
            DCHECK(i.second._offset >= 0, "Illegal Varying, name: \"%s\", offset: %d", i.first.c_str(), i.second._offset);
            _size = std::max<uint32_t>(static_cast<uint32_t>(i.second._offset) + i.second._flatable->size(), _size);
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
        _varying_builders.emplace_back(name, factory.ensureBuilderByTypeValue<Flatable>(type, value));
    }
}

sp<Varyings> Varyings::BUILDER::build(const Scope& args)
{
    if(_varying_builders.size() == 0)
        return Ark::instance().obtain<Varyings>();

    const sp<Varyings> varyings = sp<Varyings>::make();
    for(const VaryingBuilder& i : _varying_builders)
        varyings->addVarying(i._name,  i._flatable->build(args));
    return varyings;
}

template<> ARK_API sp<Varyings> Null::ptr()
{
    return Ark::instance().obtain<Varyings>();
}

Varyings::Varying::Varying(const sp<Flatable>& flatable, int32_t offset)
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

Varyings::BUILDER::VaryingBuilder::VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable)
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

}
