#include "renderer/impl/layer/gl_model_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/matrix.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_attribute.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/impl/render_command/draw_elements_instanced.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

GLModelLayer::GLModelLayer(const sp<GLModelLoader>& modelLoader, const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : Layer(resourceLoaderContext->memoryPool()),
      _resource_manager(resourceLoaderContext->glResourceManager()), _model_loader(modelLoader), _shader(shader), _atlas(atlas),
      _resource_loader_context(resourceLoaderContext), _shader_bindings(sp<GLShaderBindings>::make(_resource_manager, shader))
{
    if(atlas->texture())
        _shader_bindings->snippet()->link<GLSnippetActiveTexture>(atlas->texture());
}

const sp<Atlas>& GLModelLayer::atlas() const
{
    return _atlas;
}

sp<RenderCommand> GLModelLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    if(renderContext._items.size() > 0)
    {
        GLModelBuffer buf(_resource_loader_context, _shader_bindings, renderContext._items.size(), _shader->stride());
        const Atlas& atlas = *_atlas;
        _model_loader->start(buf, _resource_manager, renderContext);

        for(const auto& i : renderContext._items)
        {
            buf.setRenderObject(i);
            buf.setTranslate(V3(x + i._position.x(), y + i._position.y(), i._position.z()));
            _model_loader->loadModel(buf, atlas, i._type, i._size);
            if(buf.isInstanced())
            {
                GLBuffer::Builder& sBuilder = buf.getInstancedArrayBuilder(1);
                sBuilder.next();
                Matrix matrix = i._transform.toMatrix();
                matrix.translate(i._position.x(), i._position.y(), i._position.z());
                matrix.scale(i._size.x(), i._size.y(), i._size.z());
                sBuilder.write(matrix);
            }
        }
        GLDrawingContext drawingContext(_shader_bindings, _shader_bindings->arrayBuffer().snapshot(buf.vertices().makeUploader()), buf.indices(), _model_loader->mode());
        if(buf.isInstanced())
        {
            drawingContext._instanced_array_snapshots = buf.makeInstancedBufferSnapshots();
            return _resource_loader_context->objectPool()->obtain<DrawElementsInstanced>(std::move(drawingContext), _shader, renderContext._items.size());
        }
        return _resource_loader_context->objectPool()->obtain<DrawElements>(std::move(drawingContext), _shader);
    }
    return nullptr;
}

GLModelLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)), _model_loader(factory.ensureBuilder<GLModelLoader>(manifest, "model-loader")),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> GLModelLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<GLModelLayer>::make(_model_loader->build(args), _shader->build(args), atlas, _resource_loader_context);
}

}
