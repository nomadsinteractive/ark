#include "graphics/impl/renderer/renderer_with_expired.h"

#include "core/base/bean_factory.h"
#include "core/epi/disposed.h"

namespace ark {

RendererWithExpired::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _disposed(factory.ensureBuilder<Disposed>(value))
{
}

sp<Renderer> RendererWithExpired::STYLE::build(const sp<Scope>& args)
{
    sp<Renderer> bean = _delegate->build(args);
    return bean.absorb<Disposed>(_disposed->build(args));
}

RendererWithExpired::STYLE_DISPOSABLE::STYLE_DISPOSABLE(BeanFactory& /*factory*/, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate)
{
    DWARN(value.empty(), "Style \"disposable\" has unnecessary value: \"%s\"", value.c_str());
}

sp<Renderer> RendererWithExpired::STYLE_DISPOSABLE::build(const sp<Scope>& args)
{
    sp<Renderer> bean = _delegate->build(args);
    return bean.absorb<Disposed>(sp<Disposed>::make());
}

}
