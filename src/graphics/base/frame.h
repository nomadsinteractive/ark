#ifndef ARK_GRAPHICS_INF_FRAME_H_
#define ARK_GRAPHICS_INF_FRAME_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

namespace ark {

//[[core::class]]
class Frame final : public Renderer, public Block {
public:
    Frame() = default;
    Frame(const Frame& other) = default;
    Frame(Frame&& other) = default;
    Frame(const sp<Renderer>& renderer);
    Frame(const sp<Renderer>& renderer, const sp<Block>& block);
    Frame(const sp<RenderObject>& renderObject, const sp<Layer>& layer);

    explicit operator bool() const;

    Frame& operator =(const Frame& other) = default;
    Frame& operator =(Frame&& other) = default;

    const sp<Renderer>& renderer() const;

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const sp<Size>& size() override;
    const sp<Size>& size() const;

private:
    sp<Renderer> _renderer;
    sp<Block> _block;

};

}

#endif
