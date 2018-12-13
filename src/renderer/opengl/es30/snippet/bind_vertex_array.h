#ifndef ARK_RENDERER_OPENGL_ES30_SNIPPET_BIND_VERTEX_ARRAY_H_
#define ARK_RENDERER_OPENGL_ES30_SNIPPET_BIND_VERTEX_ARRAY_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {
namespace gles30 {

class BindVertexArray : public Snippet {
public:
    BindVertexArray(const sp<Resource>& vertexArray);

    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<Resource> _vertex_array;

};

}
}

#endif
