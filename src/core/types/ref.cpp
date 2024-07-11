#include "core/types/ref.h"

namespace ark {

namespace {

uint32_t hash32(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint32_t hash64(uint64_t key)
{
    key = (~key) + (key << 18);
    key = key ^ (key >> 31);
    key = key * 21;
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    return key;
}

}

Ref::Ref(const Ref& other)
    : _instance(other._instance), _instance_ref(other), _discarded(other._discarded)
{
}

Ref::~Ref()
{
    if(&_instance_ref == this)
        LOGD("Ref(%p) destroyed", toInt());
}

Ref::operator bool() const
{
    return !isDiscarded();
}

bool Ref::isDiscarded() const
{
    return _instance_ref._discarded.val();
}

const SafeVar<Boolean>& Ref::discarded() const
{
    return _instance_ref._discarded;
}

void Ref::setDiscarded(sp<Boolean> discarded)
{
    _discarded.reset(std::move(discarded));
}

void Ref::discard()
{
    _discarded.reset(true);
}

uint32_t Ref::hash() const
{
    const uintptr_t id = toInt();
    if constexpr(sizeof(uint32_t) == sizeof(uintptr_t))
        return hash32(id);
    else
        return hash64(id);
}

uintptr_t Ref::toInt() const
{
    return reinterpret_cast<uintptr_t>(&_instance_ref);
}

Ref& Ref::toRef(uintptr_t id)
{
    return *reinterpret_cast<Ref*>(id);
}

uintptr_t Ref::toInteger(const Ref& self)
{
    return self.toInt();
}

}
