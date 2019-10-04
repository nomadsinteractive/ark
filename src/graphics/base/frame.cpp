#include "graphics/base/frame.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"
#include "core/util/bean_utils.h"

#include "graphics/base/frame.h"
#include "graphics/base/render_object.h"
#include "graphics/impl/renderer/renderer_by_render_object.h"

namespace ark {

Frame::Frame(const sp<Renderer>& renderer)
    : _renderer(renderer), _block(renderer ? renderer.as<Block>() : nullptr)
{
}

Frame::Frame(const sp<Renderer>& renderer, const sp<Block>& block)
    : _renderer(renderer), _block(block)
{
}

Frame::operator bool() const
{
    return static_cast<bool>(_renderer);
}

const sp<Renderer>& Frame::renderer() const
{
    return _renderer;
}

void Frame::render(RenderRequest& renderRequest, const V3& position)
{
    DASSERT(_block);
    _renderer->render(renderRequest, position);
}

const SafePtr<Size>& Frame::size()
{
    DASSERT(_block);
    return _block->size();
}

const sp<Size>& Frame::size() const
{
    DASSERT(_block);
    return _block->size();
}

}
