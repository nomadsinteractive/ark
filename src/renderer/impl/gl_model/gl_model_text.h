#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_TEXT_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_TEXT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model.h"
#include "renderer/inf/gl_resource.h"

namespace ark {

class GLModelText : public GLModel {
private:
    class Stub : public GLResource {
    public:
        Stub(GLResourceManager& resourceManager, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);

        void reset(GLResourceManager& resourceManager, uint32_t textureWidth, uint32_t textureHeight);

        bool checkUnpreparedCharacter(const Layer::Snapshot& renderContext);
        bool prepare(const Layer::Snapshot& renderContext, bool allowReset);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext& graphicsContext) override;
        virtual void recycle(GraphicsContext& graphicsContext) override;

    private:
        bool prepareOne(int32_t c);
        void clear();

    private:
        sp<Alphabet> _alphabet;
        bitmap _font_glyph;
        sp<GLTexture> _texture;
        sp<Atlas> _atlas;
        sp<GLModel> _delegate;

        uint32_t _flowx, _flowy;
        int32_t _max_glyph_height;

        std::unordered_set<int32_t> _characters;

        friend class GLModelText;
    };

public:
    GLModelText(GLResourceManager& resourceManager, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);

    virtual void initialize(GLShaderBindings& bindings) override;
    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) override;
    virtual void load(GLModelBuffer& buf, int32_t type, const V& scale) override;
    virtual Metrics measure(int32_t type) override;

//  [[plugin::resource-loader("text")]]
    class BUILDER : public Builder<GLModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLModel> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        uint32_t _texture_width, _texture_height;
    };

private:
    sp<Stub> _stub;
};

}

#endif
