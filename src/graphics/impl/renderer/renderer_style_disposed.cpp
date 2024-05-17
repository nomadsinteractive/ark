#include "graphics/impl/renderer/renderer_style_disposed.h"

#include "core/base/bean_factory.h"
#include "core/traits/disposed.h"

namespace ark {

RendererWithExpired::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _disposed(factory.ensureBuilder<Disposed>(value))
{
}

sp<Renderer> RendererWithExpired::STYLE::build(const Scope& args)
{
    sp<Renderer> bean = _delegate->build(args);
    return bean.absorb<Disposed>(_disposed->build(args));
}

RendererWithExpired::STYLE_DISPOSABLE::STYLE_DISPOSABLE(BeanFactory& /*factory*/, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate)
{
    DCHECK_WARN(value.empty(), "Style \"disposable\" has unnecessary value: \"%s\"", value.c_str());
}

sp<Renderer> RendererWithExpired::STYLE_DISPOSABLE::build(const Scope& args)
{
    sp<Renderer> bean = _delegate->build(args);
    return bean.absorb<Disposed>(sp<Disposed>::make());
}

}
