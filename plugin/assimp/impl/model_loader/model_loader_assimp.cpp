#include "plugin/assimp/impl/model_loader/model_loader_assimp.h"

#include "core/impl/readable/bytearray_readable.h"
#include "core/inf/array.h"
#include "core/inf/loader.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/base/size.h"
#include "graphics/util/matrix_util.h"


#include "renderer/base/atlas.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/model.h"
#include "renderer/base/multi_models.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/uniform.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"
#include "renderer/inf/render_command_composer.h"

#include "assimp/impl/io/ark_io_system.h"


namespace ark {
namespace plugin {
namespace assimp {

ModelLoaderAssimp::ModelLoaderAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas, const document& manifest)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLES), _stub(sp<Stub>::make(resourceLoaderContext, atlas, manifest))
{
}

sp<RenderCommandComposer> ModelLoaderAssimp::makeRenderCommandComposer()
{
    return sp<RCCMultiDrawElementsIndirect>::make(_stub->_models);
}

void ModelLoaderAssimp::initialize(ShaderBindings& shaderBindings)
{
    for(const sp<Texture>& i : _stub->_textures)
        shaderBindings.pipelineBindings()->bindSampler(i);
}

void ModelLoaderAssimp::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderAssimp::load(int32_t type)
{
    return _stub->_models->load(type);
}

ModelLoaderAssimp::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _atlas(factory.getBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _manifest(manifest) {
}

sp<ModelLoader> ModelLoaderAssimp::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderAssimp>::make(_resource_loader_context, _atlas->build(args), _manifest);
}

ModelLoaderAssimp::Stub::Stub(const ResourceLoaderContext& resourceLoaderContext, const sp<Atlas>& atlas, const document& manifest)
    : _models(sp<MultiModels>::make())
{
    initialize(resourceLoaderContext, atlas, manifest);
}

void ModelLoaderAssimp::Stub::initialize(const ResourceLoaderContext& resourceLoaderContext, const sp<Atlas>& atlas, const document& manifest)
{
    Assimp::Importer importer;
    importer.SetIOHandler(new ArkIOSystem());

    for(const document& i : manifest->children())
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        const aiScene* scene = importer.ReadFile(src.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);
        const Rect bounds = atlas && atlas->has(type) ? atlas->getItemUV(type) : Rect(0, 1.0f, 1.0f, 0);
        _models->addModel(type, loadModel(scene, bounds));

        for(uint32_t i = 0; i < scene->mNumTextures; ++i)
            loadSceneTexture(resourceLoaderContext, scene->mTextures[i]);
    }

    importer.FreeScene();
}

Mesh ModelLoaderAssimp::Stub::loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t indexOffset) const
{
    sp<Array<element_index_t>> indices = loadIndices(mesh, indexOffset);
    sp<Array<V3>> vertices = sp<Array<V3>::Allocated>::make(mesh->mNumVertices);
    sp<Array<Mesh::UV>> uvs = sp<Array<Mesh::UV>::Allocated>::make(mesh->mNumVertices);
    sp<Array<V3>> normals = mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>::null();
    sp<Array<Mesh::Tangent>> tangents = mesh->HasTangentsAndBitangents() ? sp<Array<Mesh::Tangent>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::Tangent>::Allocated>::null();

    V3* vert = vertices->buf() - 1;
    V3* norm = normals ? normals->buf() - 1 : nullptr;
    Mesh::Tangent* t = tangents ? tangents->buf() - 1 : nullptr;
    Mesh::UV* u = uvs->buf() - 1;

    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        *(++vert) = V3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(norm)
            *(++norm) = V3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if(tangents)
            *(++t) = Mesh::Tangent(V3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z), V3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
        *(++u) = mesh->mTextureCoords[0] ? Mesh::UV(static_cast<uint16_t>((mesh->mTextureCoords[0][i].x * uvBounds.width() + uvBounds.left()) * 0xffff),
                                                    static_cast<uint16_t>((mesh->mTextureCoords[0][i].y * uvBounds.height() + uvBounds.bottom()) * 0xffff)) : Mesh::UV(0, 0);
    }

    return Mesh(indices, std::move(vertices), std::move(uvs), std::move(normals), std::move(tangents));
}

Model ModelLoaderAssimp::Stub::loadModel(const aiScene* scene, const Rect& uvBounds) const
{
    std::vector<Mesh> meshes;
    element_index_t indexOffset = 0;
    for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        meshes.push_back(loadMesh(scene->mMeshes[i], uvBounds, indexOffset));
        indexOffset += meshes.back().vertexLength();
    }

    sp<Array<Mesh>> m = sp<Array<Mesh>::Vector>::make(std::move(meshes));
    sp<Uploader> uploader = sp<IndicesUploader>::make(m);
    return Model(std::move(uploader), sp<VerticesAssimp>::make(std::move(m)));
}

bitmap ModelLoaderAssimp::Stub::loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const
{
    if(tex->mHeight == 0)
    {
        const sp<BitmapLoader>& bitmapLoader = imageResource->getLoader(tex->achFormatHint);
        return bitmapLoader->load(sp<BytearrayReadable>::make(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth)));
    }
    return bitmap::make(tex->mWidth, tex->mHeight, tex->mWidth * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth * tex->mHeight * 4));
}

void ModelLoaderAssimp::Stub::loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex)
{
    const bitmap bitmap = loadBitmap(resourceLoaderContext.bitmapBundle(), tex);
    _textures.push_back(resourceLoaderContext.renderController()->createTexture2D(sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height())), sp<Texture::UploaderBitmap>::make(bitmap)));
}

array<element_index_t> ModelLoaderAssimp::Stub::loadIndices(const aiMesh* mesh, element_index_t indexOffset) const
{
    const array<element_index_t> s = sp<Array<element_index_t>::Allocated>::make(mesh->mNumFaces * 3);
    element_index_t* buf = s->buf();
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        buf[2] = static_cast<element_index_t>(mesh->mFaces[i].mIndices[0]) + indexOffset;
        buf[1] = static_cast<element_index_t>(mesh->mFaces[i].mIndices[1]) + indexOffset;
        buf[0] = static_cast<element_index_t>(mesh->mFaces[i].mIndices[2]) + indexOffset;
        buf += 3;
    }
    return s;
}

ModelLoaderAssimp::IndicesUploader::IndicesUploader(sp<ark::Array<Mesh>> meshes)
    : Uploader(calcIndicesSize(meshes)), _meshes(std::move(meshes))
{
}

void ModelLoaderAssimp::IndicesUploader::upload(Writable& uploader)
{
    uint32_t offset = 0;
    size_t length = _meshes->length();
    Mesh* buf = _meshes->buf();
    for(size_t i = 0; i < length; ++i)
    {
        const array<element_index_t>& indices = buf[i].indices();
        uint32_t size = static_cast<uint32_t>(indices->size());
        uploader.write(indices->buf(), size, offset);
        offset += size;
    }
}

size_t ModelLoaderAssimp::IndicesUploader::calcIndicesSize(ark::Array<Mesh>& meshes) const
{
    size_t size = 0;
    size_t length = meshes.length();
    Mesh* buf = meshes.buf();
    for(size_t i = 0; i < length; ++i)
        size += buf[i].indices()->length() * sizeof(element_index_t);
    return size;
}

}
}
}
