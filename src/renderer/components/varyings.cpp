#include "renderer/components/varyings.h"

#include <ranges>

#include "core/base/allocator.h"
#include "core/base/bean_factory.h"
#include "core/base/named_hash.h"
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

    bool update(uint32_t tick) override
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
    for(const auto& i : std::views::values(shaderLayout.vertexLayouts()))
    {
        const auto [value, distance] = Math::levensteinNearest(name, i.attributes().keys());
        if(distance < nd)
            nearest = std::move(value);
    }
    return nearest;
}

String getAllAttribute(const PipelineLayout& shaderLayout)
{
    bool first = true;
    StringBuffer sb;
    for(const auto& [i, j] : shaderLayout.vertexLayouts())
        if(i != 0)
            for(const String& k : j.attributes().keys())
            {
                if(first)
                    first = false;
                else
                    sb << ", ";
                sb << "\"" << k << "\"";
            }
    return sb.str();
}

}

struct Varyings::SlotSnapshot {
    void* _content;
    uint32_t _offset;
    uint32_t _size;
    SlotSnapshot* _next = nullptr;
};

Varyings::Varyings(const Scope& kwargs)
{
    for(const auto& [k, v] : kwargs.variables())
        addVaryingProperties<float, int32_t, V2, V3, V4, V2i, V4i, M3, M4>(k, v);
}

Varyings::Varyings(const PipelineLayout& pipelineLayout)
{
    for(const auto& [k, v] : pipelineLayout.vertexLayouts())
    {
        for(const auto& [attrname, attr] : v.attributes())
            if(!(k == 0 && (attr.offset() == 0 || attr.offset() == 12)))  // slots with offset 0 and 12 in divisor 0 will always be the "a_Position" & "a_UV" attribute, which don't need to be recorded here.
                _slots.emplace(attrname, Slot(sp<Uploader>::make<UploaderSlotDefault<uint8_t>>(attr.size()), k, static_cast<int32_t>(attr.offset())));
        _slot_strides[k] = v.stride();
    }
}

bool Varyings::update(const uint32_t timestamp)
{
    bool dirty = _timestamp.update(timestamp);
    for(const auto& i : _sub_properties | std::views::values)
        if(i->update(timestamp))
            dirty = true;
    for(const auto& v: _slots | std::views::values)
        if(v._uploader->update(timestamp))
            dirty = true;
    return dirty;
}

Box Varyings::getProperty(const String& name) const
{
    const auto iter = _properties.find(Strings::capitalizeFirst(name));
    CHECK(iter != _properties.end(), "Varyings has no property \"%s\"", name.c_str());
    return iter->second;
}

Varyings::Varyings(Map<String, Slot> slots)
    : _slots(std::move(slots))
{
}

void Varyings::setSlotUploader(const String& name, sp<Uploader> uploader)
{
    const auto iter = _slots.find(name);
    CHECK(iter != _slots.end(), "Varying slot \"%s\" doesn't existing", name.c_str());
    Slot& slot = iter->second;
    CHECK(slot._uploader->size() == uploader->size(), "Replacing existing varying \"%s\"(%d) with a different size value(%d)", name.c_str(), slot._uploader->size(), uploader->size());
    slot = {std::move(uploader), slot._divisor, slot._offset};
    _timestamp.markDirty();
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

void Varyings::setProperty(const String& name, sp<Vec2i> var)
{
    setProperty<V2i>(name, std::move(var));
}

void Varyings::setProperty(const String& name, sp<Vec4i> var)
{
    setProperty<V4i>(name, std::move(var));
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
    if(_slots.empty())
    {
        Snapshot snapshot;
        snapshot.snapshotSubProperties(_sub_properties, pipelineLayout, allocator);
        return snapshot;
    }

    if(_slot_strides.empty())
    {
        for(auto& [i, j] : _slots)
        {
            Optional<const Attribute&> attr = pipelineLayout.getAttribute(i);
            CHECK(attr, "Varying has no attribute \"%s\". Did you mean \"%s\" in [%s]?", i.c_str(), findNearestAttribute(pipelineLayout, i).c_str(), getAllAttribute(pipelineLayout).c_str());
            j._divisor = attr->divisor();
            j._offset = attr->offset();
        }
        for(const auto& [k, v] : pipelineLayout.vertexLayouts())
            _slot_strides[k] = v.stride();
    }

    Array<Divided>::View buffers(reinterpret_cast<Divided*>(allocator.sbrkSpan(sizeof(Divided) * _slot_strides.size()).buf()), _slot_strides.size());

    size_t idx = 0;
    for(const auto& [i, j] : _slot_strides)
        new(&buffers.at(idx++)) Divided(i, allocator.sbrkSpan(j));

    for(const auto& [k, v] : _slots)
    {
        DASSERT(v._divisor < buffers.length());
        buffers.at(v._divisor).addSnapshot(allocator, k, v);
    }

    Snapshot snapshot(buffers);
    snapshot.snapshotSubProperties(_sub_properties, pipelineLayout, allocator);
    return snapshot;
}

Varyings::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _uploader_builders(factory.makeBuilderListObject<UploaderBuilder>(manifest, "varying"))
{
}

sp<Varyings> Varyings::BUILDER::build(const Scope& args)
{
    if(_uploader_builders.empty())
        return nullptr;

    Map<String, Slot> slots;
    for(const UploaderBuilder& i : _uploader_builders)
        slots.emplace(Strings::capitalizeFirst(i._name),  i._uploader->build(args));
    return sp<Varyings>::adopt(new Varyings(std::move(slots)));
}

Varyings::BUILDER::UploaderBuilder::UploaderBuilder(BeanFactory& factory, const document& manifest)
    : _name(Documents::ensureAttribute(manifest, constants::NAME)), _uploader(factory.ensureBuilderByTypeValue<sp<Uploader>>(Documents::ensureAttribute(manifest, constants::TYPE),
                                                                                                                               Documents::ensureAttribute(manifest, constants::VALUE)))
{
}

Varyings::Snapshot::Snapshot(const Array<Divided>::View& buffers)
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

Varyings::Divided Varyings::Snapshot::getDivided(const uint32_t divisor) const
{
    for(size_t i = 0; i < _buffers.length(); ++i)
        if(_buffers.at(i)._divisor == divisor)
            return _buffers.at(i);

    return {};
}

void Varyings::Snapshot::snapshotSubProperties(const Map<String, sp<Varyings>>& subProperties, const PipelineLayout& pipelineInput, Allocator& allocator)
{
    for(const auto& [i, j] : subProperties)
        _sub_properties[NamedHash(i).hashCode()] = j->snapshot(pipelineInput, allocator);
}

Varyings::Divided::Divided()
    : _divisor(0), _slot_snapshot(nullptr)
{
}

Varyings::Divided::Divided(const uint32_t divisor, ByteArray::View content)
    : _divisor(divisor), _content(std::move(content)), _slot_snapshot(nullptr)
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

void Varyings::Divided::addSnapshot(Allocator& allocator, const String& name, const Slot& slot)
{
    DCHECK(slot._offset >= 0, "Attribute \"%s\" has not been initialized", name.c_str());
    const uint32_t size = static_cast<uint32_t>(slot._uploader->size());
    void* content = allocator.sbrk(size);

    UploaderType::writeTo(slot._uploader, content);
    memcpy(_content.buf() + slot._offset, content, size);

    SlotSnapshot* slotSnapshot = new(allocator.sbrk(sizeof(SlotSnapshot))) SlotSnapshot{content, static_cast<uint32_t>(slot._offset), size};
    DCHECK(slotSnapshot->_offset + slotSnapshot->_size <= _content.length(), "Varyings buffer(size = %zu) overflow while adding attribute \"%s\" offset = %d size = %d ", _content.length(), name.c_str(), slotSnapshot->_offset, slotSnapshot->_size);

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

}
