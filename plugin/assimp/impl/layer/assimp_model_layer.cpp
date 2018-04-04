#include "assimp/impl/layer/assimp_model_layer.h"

#include "core/base/bean_factory.h"
#include "core/impl/array/dynamic_array.h"

#include "renderer/base/atlas.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements.h"

namespace ark {
namespace plugin {
namespace assimp {

static bytearray _loadVertices(aiMesh* mesh)
{
    const bytearray s = sp<DynamicArray<uint8_t>>::make(mesh->mNumVertices * 24);
    float* buf = reinterpret_cast<float*>(s->array());
    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        buf[0] = mesh->mVertices[i].x * 10.0f + 500;
        buf[1] = mesh->mVertices[i].y * 10.0f + 500;
        buf[2] = mesh->mVertices[i].z;
        buf[3] = mesh->mNormals[i].x;
        buf[4] = mesh->mNormals[i].y;
        buf[5] = mesh->mNormals[i].z;
        buf += 6;
    }
    return s;
}

static bytearray _loadIndices(aiMesh* mesh)
{
    const bytearray s = sp<DynamicArray<uint8_t>>::make(mesh->mNumFaces * 6);
    uint16_t* buf = reinterpret_cast<uint16_t*>(s->array());
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        buf[0] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[0]);
        buf[1] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[1]);
        buf[2] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[2]);
        buf += 3;
    }
    return s;
}

AssimpModelLayer::AssimpModelLayer(const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : Layer(resourceLoaderContext->memoryPool()), _shader(shader), _importer(sp<Assimp::Importer>::make())
{
    const aiScene* scene = _importer->ReadFile("duck.fbx", aiProcessPreset_TargetRealtime_Fast);
    _array_buffer = resourceLoaderContext->glResourceManager()->createGLBuffer(sp<Variable<bytearray>::Const>::make(_loadVertices(scene->mMeshes[0])), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    _index_buffer = resourceLoaderContext->glResourceManager()->createGLBuffer(sp<Variable<bytearray>::Const>::make(_loadIndices(scene->mMeshes[0])), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
    _snippet = resourceLoaderContext->glResourceManager()->createCoreGLSnippet(_shader, _array_buffer);
}

sp<RenderCommand> AssimpModelLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    return _render_command_pool.obtain<DrawElements>(GLSnippetContext(nullptr, _array_buffer.snapshot(nullptr), _index_buffer, GL_TRIANGLES), _shader, _snippet);
}

AssimpModelLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> AssimpModelLayer::BUILDER::build(const sp<Scope>& args)
{
    return sp<AssimpModelLayer>::make(_shader->build(args), _resource_loader_context);
}

}
}
}
