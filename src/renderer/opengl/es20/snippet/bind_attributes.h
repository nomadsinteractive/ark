#ifndef ARK_RENDERER_OPENGL_ES20_SNIPPET_BIND_ATTRIBUTES_H_
#define ARK_RENDERER_OPENGL_ES20_SNIPPET_BIND_ATTRIBUTES_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {
namespace gles20 {

class BindAttributes : public Snippet {
public:

    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;

};

}
}

#endif
