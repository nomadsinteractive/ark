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
#include "renderer/base/model_bundle.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/uniform.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"
#include "renderer/inf/render_command_composer.h"

#include "assimp/impl/animate_maker/animate_maker_assimp.h"
#include "assimp/impl/io/ark_io_system.h"


namespace ark {
namespace plugin {
namespace assimp {

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

array<element_index_t> ModelLoaderAssimp::Importer::loadIndices(const aiMesh* mesh, element_index_t vertexBase) const
{
    const array<element_index_t> s = sp<Array<element_index_t>::Allocated>::make(mesh->mNumFaces * 3);
    element_index_t* buf = s->buf();
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        const aiFace& face = mesh->mFaces[i];
        DASSERT(face.mNumIndices == 3);
        for(uint32_t j = 0; j < 3; ++j)
            *(buf++) = static_cast<element_index_t>(face.mIndices[j]) + vertexBase;
    }
    return s;
}

ModelLoaderAssimp::Importer::Importer(Ark::RendererCoordinateSystem coordinateSystem)
    : _coordinate_system(coordinateSystem)
{
}

Model ModelLoaderAssimp::Importer::import(const String& src, const Rect& uvBounds)
{
    const sp<Assimp::Importer> importer = sp<Assimp::Importer>::make();
    importer->SetIOHandler(new ArkIOSystem);
    uint32_t flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes;
    if(_coordinate_system == Ark::COORDINATE_SYSTEM_LHS)
        flags |= aiProcess_FlipWindingOrder;
    const aiScene* scene = importer->ReadFile(src.c_str(), flags);
    DCHECK(scene, "Loading \"%s\" failed", src.c_str());
    std::unordered_map<String, std::pair<size_t, aiMatrix4x4>> boneMapping;
    Model model = loadModel(scene, uvBounds, boneMapping);
    if(scene->HasAnimations())
        for(uint32_t i = 0; i < scene->mNumAnimations; ++i)
        {
            const aiAnimation* animation = scene->mAnimations[i];
            const String name = animation->mName.C_Str();
            model.animates().push_back(name, sp<AnimateMakerAssimp>::make(importer, animation, scene->mRootNode, boneMapping));
        }
    return model;
}

Mesh ModelLoaderAssimp::Importer::loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t vertexBase, std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping) const
{
    sp<Array<element_index_t>> indices = loadIndices(mesh, vertexBase);
    sp<Array<V3>> vertices = sp<Array<V3>::Allocated>::make(mesh->mNumVertices);
    sp<Array<Mesh::UV>> uvs = sp<Array<Mesh::UV>::Allocated>::make(mesh->mNumVertices);
    sp<Array<V3>> normals = mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>::null();
    sp<Array<Mesh::Tangent>> tangents = mesh->HasTangentsAndBitangents() ? sp<Array<Mesh::Tangent>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::Tangent>::Allocated>::null();
    sp<Array<Mesh::BoneInfo>> bones = mesh->HasBones() ? sp<Array<Mesh::BoneInfo>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::BoneInfo>::Allocated>::null();

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
    if(mesh->HasBones())
        loadBones(mesh, boneMapping, bones);

    return Mesh(indices, std::move(vertices), std::move(uvs), std::move(normals), std::move(tangents), std::move(bones));
}

Model ModelLoaderAssimp::Importer::loadModel(const aiScene* scene, const Rect& uvBounds, std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping) const
{
    std::vector<Mesh> meshes;
    element_index_t vertexBase = 0;

    V3 aabbMin(std::numeric_limits<float>::max()), aabbMax(std::numeric_limits<float>::min());

    for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        meshes.push_back(loadMesh(mesh, uvBounds, vertexBase, boneMapping));

        aabbMin = V3(std::min(mesh->mAABB.mMin.x, aabbMin.x()), std::min(mesh->mAABB.mMin.y, aabbMin.y()), std::min(mesh->mAABB.mMin.z, aabbMin.y()));
        aabbMax = V3(std::max(mesh->mAABB.mMax.x, aabbMax.x()), std::max(mesh->mAABB.mMax.y, aabbMax.y()), std::max(mesh->mAABB.mMax.z, aabbMax.y()));

        vertexBase += static_cast<element_index_t>(meshes.back().vertexLength());
    }

    const V3 bounds(aabbMax.x() - aabbMin.x(), aabbMax.y() - aabbMin.y(), aabbMax.z() - aabbMin.z());
    return Model(sp<Array<Mesh>::Vector>::make(std::move(meshes)), {bounds, bounds, V3(0)});
}

void ModelLoaderAssimp::Importer::loadBones(const aiMesh* mesh, std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping, Array<Mesh::BoneInfo>& bones) const
{
    Mesh::BoneInfo* bonesBuf = bones.buf();
    memset(bonesBuf, 0, bones.size());

    for(uint32_t i = 0; i < mesh->mNumBones; i++)
    {
        uint32_t index = 0;
        const String name(mesh->mBones[i]->mName.data);

        if (boneMapping.find(name) == boneMapping.end())
        {
            index = boneMapping.size();
            boneMapping.insert(std::make_pair(name, std::make_pair(index, mesh->mBones[i]->mOffsetMatrix)));
        }
        else
            index = boneMapping.at(name).first;

        for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
        {
            uint32_t vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
            Mesh::BoneInfo& boneInfo = bonesBuf[vertexID];
            boneInfo.add(index, mesh->mBones[i]->mWeights[j].mWeight);
        }
    }
}

ModelLoaderAssimp::IMPORTER_BUILDER::IMPORTER_BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _coordinate_system(resourceLoaderContext->renderController()->renderEngine()->context()->coordinateSystem())
{
}

sp<ModelBundle::Importer> ModelLoaderAssimp::IMPORTER_BUILDER::build(const Scope& /*args*/)
{
    return sp<Importer>::make(_coordinate_system);
}

}
}
}
