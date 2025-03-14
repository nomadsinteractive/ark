#include "app/util/ref_type.h"

#include "core/base/ref_manager.h"
#include "core/types/class.h"
#include "core/types/global.h"
#include "core/types/ref.h"

namespace ark {

sp<Ref> RefType::create(const RefId refid)
{
    return Global<RefManager>()->toRef(refid);
}

RefId RefType::id(const sp<Ref>& self)
{
    return self->id();
}

bool RefType::toBool(const sp<Ref>& self)
{
    return !self->isDiscarded();
}

const SafeVar<Boolean>& RefType::discarded(const sp<Ref>& self)
{
    return self->discarded();
}

void RefType::setDiscarded(const sp<Ref>& self, sp<Boolean> discarded)
{
    self->setDiscarded(std::move(discarded));
}

void RefType::discard(const sp<Ref>& self)
{
    self->discard();
}

sp<Entity> RefType::toEntity(const sp<Ref>& self)
{
    Entity& entity = self->instance<Entity>();
    return {std::shared_ptr<Entity>(&entity, [](void* ref) {}), Class::ensureClass<Entity>()};
}

}
