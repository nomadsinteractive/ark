#include "graphics/impl/renderer/renderer_with_expired.h"

#include "core/base/bean_factory.h"

namespace ark {

RendererWithExpired::DECORATOR::DECORATOR(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _expired(factory.ensureBuilder<Disposable>(value))
{
}

sp<Renderer> RendererWithExpired::DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> bean = _delegate->build(args);
    return bean.absorb<Disposable>(_expired->build(args));
}

}
