#include "renderer/base/varyings.h"

#include "core/base/allocator.h"
#include "core/base/bean_factory.h"
#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/impl/writable/writable_memory.h"
#include "core/util/math.h"
#include "core/util/uploader_type.h"

#include "graphics/base/rect.h"
#include "graphics/base/v4.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_layout.h"

namespace ark {

namespace {

template<typename T> class UploaderSlotDefault final : public Uploader {
public:
    UploaderSlotDefault(size_t length)
        : Uploader(length * sizeof(T)), _data(length) {
    }

    bool update(uint64_t timestamp) override
    {
        return false;
    }

    void upload(Writable& buf) override
    {
        buf.write(_data.data(), _data.size() * sizeof(T), 0);
    }

private:
    Vector<T> _data;
};

String findNearestAttribute(const PipelineLayout& shaderLayout, const String& name)
{
    String nearest;
    constexpr size_t nd = std::numeric_limits<size_t>::max();
    for(const auto& [i, j] : shaderLayout.streamLayouts())
    {
        const auto [value, distance] = Math::levensteinNearest(name, j.attributes().keys());
        if(distance < nd)
            nearest = std::move(value);
    }
    return nearest;
}

}

Varyings::Varyings(const PipelineLayout& pipelineLayout)
{
    for(const auto& [k, v] : pipelineLayout.streamLayouts())
    {
        for(const auto& [attrname, attr] : v.attributes())
            if(!(k == 0 && (attr.offset() == 0 || attr.offset() == 12)))  // slots with offset 0 and 12 in divisor 0 will always be the "a_Position" & "a_UV" attribute, which don't need to be recorded here.
                _slots.insert({attrname, Slot{sp<Uploader>::make<UploaderSlotDefault<uint8_t>>(attr.size()), k, static_cast<int32_t>(attr.offset())}});
        _slot_strides[k] = v.stride();
    }
}

bool Varyings::update(const uint64_t timestamp)
{
    bool dirty = _timestamp.update(timestamp);
    for(const auto& [i, j] : _sub_properties)
        if(j->update(timestamp))
            dirty = true;
    for(const auto& i : _slots)
        if(i.second._uploader->update(timestamp))
            dirty = true;
    return dirty;
}

Box Varyings::getProperty(const String& name) const
{
    const auto iter = _properties.find(Strings::capitalizeFirst(name));
    CHECK(iter != _properties.end(), "Varyings has no property \"%s\"", name.c_str());
    return iter->second;
}

void Varyings::setSlotInput(const String& name, sp<Uploader> input)
{
    if(const auto iter = _slots.find(name); iter == _slots.end())
    {
        _slots.emplace(name, Slot{std::move(input)});
        _slot_strides.clear();
    }
    else
    {
        const Slot& preslot = iter->second;
        CHECK(preslot._uploader->size() == input->size(), "Replacing existing varying \"%s\"(%d) with a different size value(%d)", name.c_str(), preslot._uploader->size(), input->size());
        iter->second = {std::move(input), preslot._divisor, preslot._offset};
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

Varyings::Snapshot Varyings::snapshot(const PipelineLayout& pipelineLayout, Allocator& allocator)
{
    if(!_slots.size())
    {
        Snapshot snapshot;
        snapshot.snapshotSubProperties(_sub_properties, pipelineLayout, allocator);
        return snapshot;
    }

    if(!_slot_strides.size())
    {
        for(auto& [i, j] : _slots)
        {
            Optional<const Attribute&> attr = pipelineLayout.getAttribute(i);
            CHECK(attr, "Varying has no attribute \"%s\". Did you mean \"%s\"?", i.c_str(), findNearestAttribute(pipelineLayout, i).c_str());
            j._divisor = attr->divisor();
            j._offset = attr->offset();
        }
        for(const auto& [k, v] : pipelineLayout.streamLayouts())
            _slot_strides[k] = v.stride();
    }

    Array<Divided>::Borrowed buffers(reinterpret_cast<Divided*>(allocator.sbrkSpan(sizeof(Divided) * _slot_strides.size()).buf()), _slot_strides.size());

    size_t idx = 0;
    for(const auto& [i, j] : _slot_strides)
        new(&buffers.at(idx++)) Divided(i, allocator.sbrkSpan(j));

    for(const auto& [i, j] : _slots)
    {
        DASSERT(j._divisor < buffers.length());
        buffers.at(j._divisor).addSnapshot(allocator, j);
    }

    Snapshot snapshot(buffers);
    snapshot.snapshotSubProperties(_sub_properties, pipelineLayout, allocator);
    return snapshot;
}

Varyings::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _input_builders(factory.makeBuilderListObject<InputBuilder>(manifest, "varying"))
{
}

sp<Varyings> Varyings::BUILDER::build(const Scope& args)
{
    if(_input_builders.empty())
        return nullptr;

    const sp<Varyings> varyings = sp<Varyings>::make();
    for(const InputBuilder& i : _input_builders)
        varyings->setSlotInput(i._name,  i._input->build(args));
    return varyings;
}

Varyings::BUILDER::InputBuilder::InputBuilder(BeanFactory& factory, const document& manifest)
    : _name(Documents::ensureAttribute(manifest, constants::NAME)), _input(factory.ensureBuilderByTypeValue<Uploader>(Documents::ensureAttribute(manifest, constants::TYPE),
                                                                                                                               Documents::ensureAttribute(manifest, constants::VALUE)))
{
}

Varyings::Snapshot::Snapshot(const Array<Divided>::Borrowed& buffers)
    : _buffers(buffers)
{
}

void Varyings::Snapshot::apply(const Snapshot* defaults)
{
    for(size_t i = 0; i < _buffers.length(); ++i)
    {
        Divided& div = _buffers.at(i);
        if(defaults)
            if(const Divided def = defaults->getDivided(div._divisor))
                div.apply(def._slot_snapshot);
        div.apply();
    }
}

Varyings::Snapshot::operator bool() const
{
    return _buffers.buf() != nullptr;
}

Varyings::Divided Varyings::Snapshot::getDivided(uint32_t divisor) const
{
    for(size_t i = 0; i < _buffers.length(); ++i)
        if(_buffers.at(i)._divisor == divisor)
            return _buffers.at(i);

    return Varyings::Divided();
}

void Varyings::Snapshot::snapshotSubProperties(const Map<String, sp<Varyings>>& subProperties, const PipelineLayout& pipelineInput, Allocator& allocator)
{
    for(const auto& [i, j] : subProperties)
        _sub_properties[i.hash()] = j->snapshot(pipelineInput, allocator);
}

Varyings::Divided::Divided()
    : _divisor(0), _slot_snapshot(nullptr)
{
}

Varyings::Divided::Divided(uint32_t divisor, ByteArray::Borrowed content)
    : _divisor(divisor), _content(content), _slot_snapshot(nullptr)
{
}

Varyings::Divided::operator bool() const
{
    return _content.length() > 0;
}

void Varyings::Divided::apply(const SlotSnapshot* slots)
{
    uint8_t* ptr = _content.buf();

    const SlotSnapshot* iter = slots ? slots : _slot_snapshot;
    while(iter)
    {
        memcpy(ptr + iter->_offset, iter->_content, iter->_size);
        iter = iter->_next;
    }
}

void Varyings::Divided::addSnapshot(Allocator& allocator, const Slot& slot)
{
    DASSERT(slot._offset >= 0);
    const uint32_t size = static_cast<uint32_t>(slot._uploader->size());
    void* content = allocator.sbrk(size);

    UploaderType::writeTo(slot._uploader, content);
    memcpy(_content.buf() + slot._offset, content, size);
    addSlotSnapshot(new(allocator.sbrk(sizeof(SlotSnapshot))) SlotSnapshot(content, slot._offset, size));
}

void Varyings::Divided::addSlotSnapshot(SlotSnapshot* slotSnapshot)
{
    DASSERT(slotSnapshot->_offset + slotSnapshot->_size <= _content.length());

    if(!_slot_snapshot)
    {
        _slot_snapshot = slotSnapshot;
        return;
    }

    SlotSnapshot* tail = _slot_snapshot;
    while(tail->_next != nullptr)
        tail = tail->_next;
    tail->_next = slotSnapshot;
}

Varyings::SlotSnapshot::SlotSnapshot(void* content, uint32_t offset, uint32_t size)
    : _content(content), _offset(offset), _size(size), _next(nullptr)
{
}

}
