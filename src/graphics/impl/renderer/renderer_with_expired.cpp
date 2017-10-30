#include "graphics/impl/renderer/renderer_with_expired.h"

#include "core/base/bean_factory.h"

namespace ark {

RendererWithExpired::DECORATOR::DECORATOR(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _expired(factory.ensureBuilder<Expired>(value))
{
}

sp<Renderer> RendererWithExpired::DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> bean = _delegate->build(args);
    return bean.absorb<Expired>(_expired->build(args));
}

}
