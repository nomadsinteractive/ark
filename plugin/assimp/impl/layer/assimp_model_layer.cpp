#include "assimp/impl/layer/assimp_model_layer.h"

#include "core/base/bean_factory.h"
#include "core/inf/loader.h"
#include "core/impl/array/dynamic_array.h"
#include "core/impl/array/preallocated_array.h"
#include "core/impl/readable/bytearray_readable.h"

#include "graphics/base/image_resource.h"
#include "graphics/base/bitmap.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_attribute.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements_3d.h"

#include "assimp/impl/io/ark_io_system.h"

namespace ark {
namespace plugin {
namespace assimp {

namespace {

class AssimpGLBufferUploader : public GLBuffer::Uploader {
public:
    AssimpGLBufferUploader(const bytearray& buffer)
        : _buffer(buffer) {
    }

    virtual size_t size() override {
        return _buffer->length();
    }

    virtual void upload(GraphicsContext& /*graphicsContext*/, GLenum target, GLsizeiptr size) override {
        glBufferSubData(target, 0, _buffer->length(), _buffer->array());
    }

private:
    bytearray _buffer;
};

}

AssimpModelLayer::AssimpModelLayer(const sp<GLShader>& shader, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : Layer(resourceLoaderContext->memoryPool()), _shader(shader), _importer(sp<Assimp::Importer>::make())
{
    _importer->SetIOHandler(new ArkIOSystem());
    const aiScene* scene = _importer->ReadFile("duck.fbx", aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs);
    const bytearray vertices = loadArrayBuffer(scene->mMeshes[0], 1.5f);
    const bytearray indices = loadIndexBuffer(scene->mMeshes[0]);
    const bitmap tex = loadBitmap(resourceLoaderContext, scene->mTextures[0]);
    _array_buffer = resourceLoaderContext->glResourceManager()->createGLBuffer(sp<AssimpGLBufferUploader>::make(vertices), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    _index_buffer = resourceLoaderContext->glResourceManager()->createGLBuffer(sp<AssimpGLBufferUploader>::make(indices), GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    _texture = resourceLoaderContext->glResourceManager()->createGLTexture(tex->width(), tex->height(), sp<Variable<bitmap>::Const>::make(tex));
    _snippet = resourceLoaderContext->glResourceManager()->createCoreGLSnippet(_shader, _array_buffer);
    resourceLoaderContext->glResourceManager()->prepare(_array_buffer, GLResourceManager::PS_ONCE_AND_ON_SURFACE_READY);
    resourceLoaderContext->glResourceManager()->prepare(_index_buffer, GLResourceManager::PS_ONCE_AND_ON_SURFACE_READY);
}

sp<RenderCommand> AssimpModelLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    return _render_command_pool.obtain<DrawElements3D>(GLSnippetContext(_texture, _array_buffer.snapshot(nullptr), _index_buffer, GL_TRIANGLES), _shader, _snippet);
}

bytearray AssimpModelLayer::loadArrayBuffer(aiMesh* mesh, float scale) const
{
    const bytearray s = sp<DynamicArray<uint8_t>>::make(mesh->mNumVertices * _shader->stride());
    uint8_t* buf = s->array();
    const GLAttribute& aPosition = _shader->getAttribute("Position");
    const GLAttribute& aNormal = _shader->getAttribute("Normal");
    const GLAttribute& aTexCoordinate = _shader->getAttribute("TexCoordinate");
    uint32_t aps = aPosition.offset() / 4;
    uint32_t ans = aNormal.offset() / 4;
    uint32_t ats = aTexCoordinate.offset() / 2;
    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        float* fptr = reinterpret_cast<float*>(buf);
        uint16_t* sptr = reinterpret_cast<uint16_t*>(buf);
        fptr[aps] = mesh->mVertices[i].x * scale + 200;
        fptr[aps + 1] = mesh->mVertices[i].y * scale + 500;
        fptr[aps + 2] = mesh->mVertices[i].z * scale;
        fptr[ans] = mesh->mNormals[i].x;
        fptr[ans + 1] = mesh->mNormals[i].y;
        fptr[ans + 2] = mesh->mNormals[i].z;
        sptr[ats] = static_cast<uint16_t>(mesh->mTextureCoords[0][i].x * 0xffff);
        sptr[ats + 1] = static_cast<uint16_t>(mesh->mTextureCoords[0][i].y * 0xffff);
        buf += _shader->stride();
    }
    return s;
}

bytearray AssimpModelLayer::loadIndexBuffer(aiMesh* mesh) const
{
    const bytearray s = sp<DynamicArray<uint8_t>>::make(mesh->mNumFaces * 6);
    uint16_t* buf = reinterpret_cast<uint16_t*>(s->array());
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        buf[2] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[0]);
        buf[1] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[1]);
        buf[0] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[2]);
        buf += 3;
    }
    return s;
}

bitmap AssimpModelLayer::loadBitmap(const sp<ResourceLoaderContext>& resourceLoaderContext, aiTexture* tex) const
{
    if(tex->mHeight == 0)
    {
        const sp<BitmapLoader> bitmapLoader = resourceLoaderContext->imageResource()->getLoader(tex->achFormatHint);
        return bitmapLoader->load(sp<BytearrayReadable>::make(sp<PreallocatedArray<uint8_t>>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth)));
    }
    return bitmap::make(tex->mWidth, tex->mHeight, tex->mWidth * 4, 4, sp<PreallocatedArray<uint8_t>>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth * tex->mHeight * 4));
}

AssimpModelLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> AssimpModelLayer::BUILDER::build(const sp<Scope>& args)
{
    return sp<AssimpModelLayer>::make(_shader->build(args), _manifest, _resource_loader_context);
}

}
}
}