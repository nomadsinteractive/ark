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
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/model_bundle.h"
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

ModelLoaderAssimp::ModelLoaderAssimp(sp<Atlas> atlas, const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLES), _model_bundle(makeModelBundle(resourceLoaderContext, manifest, std::move(atlas)))
{

}

sp<RenderCommandComposer> ModelLoaderAssimp::makeRenderCommandComposer()
{
    return sp<RCCMultiDrawElementsIndirect>::make(_model_bundle);
}

void ModelLoaderAssimp::initialize(ShaderBindings& /*shaderBindings*/)
{
}

void ModelLoaderAssimp::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderAssimp::load(int32_t type)
{
    return _model_bundle->load(type);
}

ModelLoaderAssimp::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.getBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _manifest(manifest), _resource_loader_context(resourceLoaderContext) {
}

sp<ModelLoader> ModelLoaderAssimp::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderAssimp>::make(_atlas->build(args), _resource_loader_context, _manifest);
}

bitmap ModelLoaderAssimp::loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const
{
    if(tex->mHeight == 0)
    {
        const sp<BitmapLoader>& bitmapLoader = imageResource->getLoader(tex->achFormatHint);
        return bitmapLoader->load(sp<BytearrayReadable>::make(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth)));
    }
    return bitmap::make(tex->mWidth, tex->mHeight, tex->mWidth * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth * tex->mHeight * 4));
}

void ModelLoaderAssimp::loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex)
{
    const bitmap bitmap = loadBitmap(resourceLoaderContext.bitmapBundle(), tex);
    _textures.push_back(resourceLoaderContext.renderController()->createTexture2D(sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height())), sp<Texture::UploaderBitmap>::make(bitmap)));
}

sp<ModelBundle> ModelLoaderAssimp::makeModelBundle(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest, sp<Atlas> atlas)
{
    Importer importer;
    sp<ModelBundle> modelBundle = sp<ModelBundle>::make(std::move(atlas));
    modelBundle->import(resourceLoaderContext, manifest, importer);
    return modelBundle;
}

array<element_index_t> ModelLoaderAssimp::Importer::loadIndices(const aiMesh* mesh, element_index_t indexOffset) const
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

ModelLoaderAssimp::Importer::Importer()
{
    _importer.SetIOHandler(new ArkIOSystem());
}

Model ModelLoaderAssimp::Importer::import(const String& src, const Rect& bounds)
{
    const aiScene* scene = _importer.ReadFile(src.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);
    return loadModel(scene, bounds);
}

Mesh ModelLoaderAssimp::Importer::loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t indexOffset) const
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

Model ModelLoaderAssimp::Importer::loadModel(const aiScene* scene, const Rect& uvBounds) const
{
    std::vector<Mesh> meshes;
    element_index_t indexOffset = 0;
    for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        meshes.push_back(loadMesh(scene->mMeshes[i], uvBounds, indexOffset));
        indexOffset += meshes.back().vertexLength();
    }

    return Model(sp<Array<Mesh>::Vector>::make(std::move(meshes)));
}

}
}
}
