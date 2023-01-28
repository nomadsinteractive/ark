#include "renderer/base/varyings.h"

#include "core/ark.h"
#include "core/base/allocator.h"
#include "core/base/bean_factory.h"
#include "core/inf/input.h"
#include "core/inf/variable.h"
#include "core/impl/writable/writable_memory.h"
#include "core/util/holder_util.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/base/v4.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/varyings.h"

namespace ark {

Varyings::Varyings()
{
}

void Varyings::traverse(const Holder::Visitor& visitor)
{
    for(const auto& iter : _slots)
        HolderUtil::visit(iter.second._input, visitor);
}

bool Varyings::update(uint64_t timestamp)
{
    bool dirty = _timestamp.update(timestamp);
    for(const auto& [i, j] : _sub_properties)
        if(j->update(timestamp))
            dirty = true;
    for(const auto& i : _slots)
        if(i.second._input->update(timestamp))
            dirty = true;
    return dirty;
}

Box Varyings::getProperty(const String& name) const
{
    const auto iter = _properties.find(Strings::capitalizeFirst(name));
    CHECK(iter != _properties.end(), "Varyings has no property \"%s\"", name.c_str());
    return iter->second;
}

void Varyings::setSlotInput(const String& name, sp<Input> input)
{
    const auto iter = _slots.find(name);
    if(iter == _slots.end())
    {
        _slots.emplace(name, std::move(input));
        _slot_strides.clear();
    }
    else
    {
        Slot& preslot = iter->second;
        CHECK(preslot._input->size() == input->size(), "Replacing existing varying \"%s\"(%d) with a different size value(%d)", name.c_str(), preslot._input->size(), input->size());
        iter->second = Slot(std::move(input), preslot._divisor, preslot._offset);
    }
}

void Varyings::setProperty(const String& name, sp<Integer> var)
{
    setProperty<int32_t>(name, std::move(var));
}

void Varyings::setProperty(const String& name, sp<Numeric> var)
{
    setProperty<float>(name, std::move(var));
}

void Varyings::setProperty(const String& name, sp<Vec2> var)
{
    setProperty<V2>(name, std::move(var));
}

void Varyings::setProperty(const String& name, sp<Vec3> var)
{
    setProperty<V3>(name, std::move(var));
}

void Varyings::setProperty(const String& name, sp<Vec4> var)
{
    setProperty<V4>(name, std::move(var));
}

void Varyings::setProperty(const String& name, sp<Mat4> var)
{
    setProperty<M4>(name, std::move(var));
}

sp<Varyings> Varyings::subscribe(const String& name)
{
    sp<Varyings>& subProp = _sub_properties[name];
    if(!subProp)
        subProp = sp<Varyings>::make();
    return subProp;
}

static String findNearestAttribute(const PipelineInput& pipelineInput, const String& name)
{
    String nearest;
    size_t nd = std::numeric_limits<size_t>::max();
    for(const auto& [i, j] : pipelineInput.streams())
    {
        const auto [value, distance] = Math::levensteinNearest(name, j.attributes().keys());
        if(distance < nd)
            nearest = std::move(value);
    }
    return nearest;
}

Varyings::Snapshot Varyings::snapshot(const PipelineInput& pipelineInput, Allocator& allocator)
{
    if(!_slots.size())
    {
        Snapshot snapshot;
        snapshot.snapshotSubProperties(_sub_properties, pipelineInput, allocator);
        return snapshot;
    }

    if(!_slot_strides.size())
    {
        for(auto& [i, j] : _slots)
        {
            Optional<const Attribute&> attr = pipelineInput.getAttribute(i);
            CHECK(attr, "Varying has no attribute \"%s\". Did you mean \"%s\"?", i.c_str(), findNearestAttribute(pipelineInput, i).c_str());
            j._divisor = attr->divisor();
            j._offset = attr->offset();
        }
        for(const auto& i : pipelineInput.streams())
            _slot_strides[i.first] = i.second.stride();
    }

    Array<Divided>::Borrowed buffers(reinterpret_cast<Divided*>(allocator.sbrk(sizeof(Divided) * _slot_strides.size()).buf()), _slot_strides.size());

    size_t idx = 0;
    for(const auto& [i, j] : _slot_strides)
        new(&buffers.at(idx++)) Divided(i, allocator.sbrk(j));

    for(const auto& [i, j] : _slots)
    {
        DASSERT(j._divisor < buffers.length());
        j.apply(buffers.at(j._divisor)._content.buf());
    }

    Snapshot snapshot(buffers);
    snapshot.snapshotSubProperties(_sub_properties, pipelineInput, allocator);
    return snapshot;
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
        varyings->setSlotInput(i._name,  i._input->build(args));
    return varyings;
}

template<> ARK_API sp<Varyings> Null::safePtr()
{
    return sp<Varyings>::make();
}

Varyings::Slot::Slot(sp<Input> input, uint32_t divisor, int32_t offset)
    : _input(std::move(input)), _divisor(divisor), _offset(offset)
{
}

void Varyings::Slot::apply(uint8_t* ptr) const
{
    DASSERT(_offset >= 0);
    WritableMemory writer(ptr + _offset);
    _input->upload(writer);
}

Varyings::BUILDER::VaryingBuilder::VaryingBuilder(String name, sp<Builder<Input>> input)
    : _name(std::move(name)), _input(std::move(input))
{
}

Varyings::Snapshot::Snapshot(Array<Varyings::Divided>::Borrowed buffers)
    : _buffers(buffers)
{
}

Varyings::Snapshot::operator bool() const
{
    return _buffers.buf() != nullptr;
}

ByteArray::Borrowed Varyings::Snapshot::getDivided(uint32_t divisor) const
{
    for(size_t i = 0; i < _buffers.length(); ++i)
        if(_buffers.at(i)._divisor == divisor)
            return _buffers.at(i)._content;
    return ByteArray::Borrowed();
}

void Varyings::Snapshot::snapshotSubProperties(const std::map<String, sp<Varyings>>& subProperties, const PipelineInput& pipelineInput, Allocator& allocator)
{
    for(const auto& [i, j] : subProperties)
        _sub_properties[i.hash()] = j->snapshot(pipelineInput, allocator);
}

Varyings::Divided::Divided(uint32_t divisor, ByteArray::Borrowed content)
    : _divisor(divisor), _content(content)
{
}

}
