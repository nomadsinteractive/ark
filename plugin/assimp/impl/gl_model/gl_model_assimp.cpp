#include "plugin/assimp/impl/gl_model/gl_model_assimp.h"

#include "core/impl/readable/bytearray_readable.h"
#include "core/inf/array.h"
#include "core/inf/loader.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"

#include "renderer/base/model_buffer.h"
#include "renderer/base/resource_loader_context.h"

#include "assimp/impl/io/ark_io_system.h"

namespace ark {
namespace plugin {
namespace assimp {

GLModelAssimp::GLModelAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest)
    :_importer(sp<Assimp::Importer>::make())
{
    _importer->SetIOHandler(new ArkIOSystem());

    for(const document& i : manifest->children())
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        const aiScene* scene = _importer->ReadFile(src.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs);
        _models[type] = sp<Mesh>::make(scene->mMeshes[0]);
    }

    _importer->FreeScene();
//    const bytearray vertices = loadMesh(scene->mMeshes[0], 1.5f);
//    const bytearray indices = loadIndexBuffer(scene->mMeshes[0]);
//    const bitmap tex = loadBitmap(resourceLoaderContext, scene->mTextures[0]);
//    _array_buffer = resourceLoaderContext->glResourceManager()->makeGLBuffer(sp<AssimpGLBufferUploader>::make(vertices), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
//    _index_buffer = resourceLoaderContext->glResourceManager()->makeGLBuffer(sp<AssimpGLBufferUploader>::make(indices), GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
//    const sp<GLTexture> texture = resourceLoaderContext->glResourceManager()->createGLTexture(tex->width(), tex->height(), sp<Variable<bitmap>::Const>::make(tex));
//    _shader_bindings = sp<ShaderBindings>::make(shader, _array_buffer);
//    _shader_bindings = sp<ShaderBindings>::make(resourceLoaderContext->glResourceManager(), shader);
//    const sp<GLSnippetTextures> textures = _shader_bindings->snippet()->link<GLSnippetTextures>();
//    textures->addTexture(0, texture);
//    _shader_bindings->snippet()->link<GLSnippetUpdateModelMatrix>();
}

sp<ShaderBindings> GLModelAssimp::makeShaderBindings(const Shader& shader)
{
    return nullptr;
}

void GLModelAssimp::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

void GLModelAssimp::start(ModelBuffer& buf, const RenderLayer::Snapshot& layerContext)
{
}

void GLModelAssimp::load(ModelBuffer& buf, const RenderObject::Snapshot& snapshot)
{
}

bitmap GLModelAssimp::loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const
{
    if(tex->mHeight == 0)
    {
        const sp<BitmapLoader>& bitmapLoader = imageResource->getLoader(tex->achFormatHint);
        return bitmapLoader->load(sp<BytearrayReadable>::make(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth)));
    }
    return bitmap::make(tex->mWidth, tex->mHeight, tex->mWidth * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth * tex->mHeight * 4));
}

GLModelAssimp::BUILDER::BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _manifest(manifest) {
}

sp<RenderModel> GLModelAssimp::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelAssimp>::make(_resource_loader_context, _manifest);
}

GLModelAssimp::Mesh::Mesh(const aiMesh* mesh)
    : _vertices(sp<Array<V3>::Allocated>::make(mesh->mNumVertices)),
      _normals(mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>::null()),
      _tangents(mesh->HasTangentsAndBitangents() ? sp<Array<std::pair<V3, V3>>::Allocated>::make(mesh->mNumVertices) : sp<Array<std::pair<V3, V3>>::Allocated>::null()),
      _uvs(sp<Array<UV>::Allocated>::make(mesh->mNumVertices)), _indices(loadIndices(mesh))
{
    V3* vertices = _vertices->buf();
    V3* normals = _normals ? _normals->buf() : nullptr;
    std::pair<V3, V3>* tangents = _tangents ? _tangents->buf() : nullptr;
    UV* uvs = _uvs->buf();
    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        *(vertices++) = V3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(normals)
            *(normals++) = V3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if(tangents)
            *(tangents++) = std::make_pair(V3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z), V3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
        *(uvs++) = UV(static_cast<uint16_t>(mesh->mTextureCoords[0][i].x * 0xffff), static_cast<uint16_t>(mesh->mTextureCoords[0][i].y * 0xffff));
    }
}

void GLModelAssimp::Mesh::load(ModelBuffer& buf) const
{
    V3* vertices = _vertices->buf();
    V3* normals = _normals ? _normals->buf() : nullptr;
    std::pair<V3, V3>* tangents = _tangents ? _tangents->buf() : nullptr;
    UV* uvs = _uvs->buf();

    size_t length = _vertices->length();
    for(size_t i = 0; i < length; ++i)
    {
        buf.writePosition(*(vertices++));
        if(normals)
            buf.writeNormal(*(normals++));
        if(tangents)
        {
            buf.writeTangent(tangents->first);
            ++tangents;
        }
        buf.writeTexCoordinate(uvs->u, uvs->v);
        ++uvs;
    }
}

bytearray GLModelAssimp::Mesh::loadIndices(const aiMesh* mesh) const
{
    const bytearray s = sp<ByteArray::Allocated>::make(mesh->mNumFaces * 6);
    uint16_t* buf = reinterpret_cast<uint16_t*>(s->buf());
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        buf[2] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[0]);
        buf[1] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[1]);
        buf[0] = static_cast<uint16_t>(mesh->mFaces[i].mIndices[2]);
        buf += 3;
    }
    return s;
}

GLModelAssimp::UV::UV()
    : u(0), v(0)
{
}

GLModelAssimp::UV::UV(uint16_t u, uint16_t v)
    : u(u), v(v)
{
}

}
}
}
