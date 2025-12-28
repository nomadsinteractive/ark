#include "core/base/artifact.h"

#include "core/impl/uploader/uploader_of_variable.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/writable/writable_memory.h"
#include "core/impl/writable/writable_with_offset.h"

#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

template<typename T, typename... Args> sp<Uploader> makeProperty(const String& name, const Box& value)
{
    if(sp<Uploader> prop = value.as<Uploader>())
        return prop;

    if(sp<Variable<T>> prop = value.as<Variable<T>>())
        return sp<Uploader>::make<UploaderOfVariable<T>>(std::move(prop));

    if constexpr(sizeof...(Args) > 0)
        return makeProperty<Args...>(name, value);
    FATAL("Cannot make Artifact property \"%s\", all variable option types tried out.", name.c_str());
    return nullptr;
}

class UploaderArtifactRepeat final : public Uploader {
public:
    UploaderArtifactRepeat(sp<Artifact> artifact, const uint32_t repeatCount, sp<IntegerWrapper> index)
        : Uploader(artifact->size() * repeatCount), _artifact(std::move(artifact)), _repeat_count(repeatCount), _index(std::move(index)), _base_index(_index ? _index->val() : 0), _buffer(_size) {
    }

    bool update(const uint32_t tick) override
    {
        return _artifact->updateRepeat(tick, _repeat_count, _buffer, _index, _base_index);
    }

    void upload(Writable& buf) override
    {
        buf.write(_buffer.data(), _size, 0);
    }

private:
    sp<Artifact> _artifact;
    uint32_t _repeat_count;
    sp<IntegerWrapper> _index;
    int32_t _base_index;
    Vector<int8_t> _buffer;
};

}

Artifact::Artifact(const Scope& kwargs)
    : Uploader(0)
{
    _size = loadProperties(kwargs);
}

bool Artifact::update(const uint32_t tick)
{
    bool dirty = false;
    for(const auto& [_, i] : _properties.values())
        dirty = i->update(tick) || dirty;
    return dirty;
}

void Artifact::upload(Writable& buf)
{
    for(const auto& [offset, uploader] : _properties.values())
    {
        WritableWithOffset writable(buf, offset);
        uploader->upload(writable);
    }
}

sp<Uploader> Artifact::makeUploader(sp<Artifact> artifact, uint32_t repeatCount, sp<IntegerWrapper> index)
{
    return sp<Uploader>::make<UploaderArtifactRepeat>(std::move(artifact), repeatCount, std::move(index));
}

bool Artifact::updateRepeat(const uint32_t tick, const uint32_t repeatCount, Vector<int8_t>& buffer, const sp<IntegerWrapper>& index, const int32_t baseIndex)
{
    bool dirty = false;
    int8_t* ptr = buffer.data();
    for(int32_t i = 0; i < repeatCount; ++i)
    {
        if(index)
            index->set(i + baseIndex);
        for(const auto& [offset, i] : _properties.values())
        {
            WritableMemory writable(ptr + offset);
            dirty = i->update(tick) || dirty;
            i->upload(writable);
        }
        ptr += _size;
    }
    return dirty;
}

size_t Artifact::loadProperties(const Scope& kwargs)
{
    size_t size = 0;
    for(const auto& [k, v] : kwargs.variables())
    {
        const uint32_t offset = size;
        sp<Uploader> uploader = makeProperty<float, int32_t, V2, V3, V4, V2i, V4i, M3, M4>(k, v);
        size += uploader->size();
        _properties.push_back(k, {offset, std::move(uploader)});
    }
    return size;
}

}
