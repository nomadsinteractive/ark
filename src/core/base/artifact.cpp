#include "core/base/artifact.h"

#include "core/impl/uploader/uploader_of_variable.h"
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
    UploaderArtifactRepeat(sp<Artifact> artifact, const uint32_t repeat)
        : Uploader(artifact->size() * repeat), _artifact(std::move(artifact)), _repeat_count(repeat), _buffer(_size) {
    }

    bool update(const uint32_t tick) override
    {
        return _artifact->updateRepeat(tick, _repeat_count, _buffer);
    }

    void upload(Writable& buf) override
    {
        buf.write(_buffer.data(), _size, 0);
    }

private:
    sp<Artifact> _artifact;
    uint32_t _repeat_count;
    Vector<int8_t> _buffer;
};

}

class Artifact::Index final : public Integer {
public:
    Index()
        : _value(0) {
    }

    bool update(uint32_t tick) override
    {
        return true;
    }

    int32_t val() override
    {
        return _value;
    }

    int32_t _value;
};

Artifact::Artifact(const Scope& kwargs)
    : Uploader(loadProperties(kwargs)), _index(sp<Index>::make())
{
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

sp<Integer> Artifact::index() const
{
    return _index;
}

sp<Uploader> Artifact::mul(sp<Artifact> lhs, const uint32_t rhs)
{
    return sp<Uploader>::make<UploaderArtifactRepeat>(std::move(lhs), rhs);
}

bool Artifact::updateRepeat(const uint32_t tick, const uint32_t repeatCount, Vector<int8_t>& buffer)
{
    bool dirty = false;
    int8_t* ptr = buffer.data();
    for(uint32_t i = 0; i < repeatCount; ++i)
    {
        WritableMemory writable(ptr + _size * i);
        _index->_value = i;
        for(const auto& [_, i] : _properties.values())
        {
            dirty = i->update(tick) || dirty;
            i->upload(writable);
        }
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
