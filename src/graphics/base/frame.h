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
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Frame);
    Frame(const sp<Renderer>& renderer);
    Frame(const sp<Renderer>& renderer, const sp<Block>& block);

    explicit operator bool() const;

    const sp<Renderer>& renderer() const;

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual const SafePtr<Size>& size() override;
    const sp<Size>& size() const;

private:
    sp<Renderer> _renderer;
    sp<Block> _block;

};

}

#endif
