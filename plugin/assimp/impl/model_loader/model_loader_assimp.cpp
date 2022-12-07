#include "plugin/assimp/impl/model_loader/model_loader_assimp.h"

#include "core/base/manifest.h"
#include "core/impl/readable/bytearray_readable.h"
#include "core/inf/array.h"
#include "core/inf/loader.h"
#include "core/util/documents.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/material.h"
#include "graphics/base/size.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/material_bundle.h"
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

#include "assimp/base/node_table.h"
#include "assimp/impl/animation/animation_assimp_nodes.h"
#include "assimp/impl/io/ark_io_system.h"


namespace ark {
namespace plugin {
namespace assimp {

bitmap ModelImporterAssimp::loadBitmap(const sp<BitmapLoaderBundle>& imageResource, const aiTexture* tex) const
{
    if(tex->mHeight == 0)
    {
        const sp<BitmapLoader>& bitmapLoader = imageResource->getLoader(tex->achFormatHint);
        return bitmapLoader->load(sp<BytearrayReadable>::make(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth)));
    }
    return bitmap::make(tex->mWidth, tex->mHeight, tex->mWidth * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth * tex->mHeight * 4));
}

void ModelImporterAssimp::loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex)
{
    bitmap bitmap = loadBitmap(resourceLoaderContext.bitmapBundle(), tex);
    _textures.push_back(resourceLoaderContext.renderController()->createTexture2D(sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height())), std::move(bitmap)));
}

std::vector<element_index_t> ModelImporterAssimp::loadIndices(const aiMesh* mesh, element_index_t vertexBase) const
{
    std::vector<element_index_t> s(mesh->mNumFaces * 3);
    element_index_t* buf = s.data();
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        const aiFace& face = mesh->mFaces[i];
        DASSERT(face.mNumIndices <= 3);
        for(uint32_t j = 0; j < 3; ++j)
            *(buf++) = face.mNumIndices == 3 ? static_cast<element_index_t>(face.mIndices[j]) + vertexBase : 0;
    }
    return s;
}

void ModelImporterAssimp::loadNodeHierarchy(const aiNode* node, NodeTable& nodes, std::unordered_map<uint32_t, uint32_t>& nodeIds) const
{
    if(node->mNumMeshes)
    {
        const String nodeName(node->mName.C_Str());
        const Node& n = nodes.ensureNode(nodeName, node->mTransformation);
        for(uint32_t i = 0; i < node->mNumMeshes; ++i)
            nodeIds[node->mMeshes[i]] = n._id;
    }

    for(uint32_t i = 0; i < node->mNumChildren; ++i)
        loadNodeHierarchy(node->mChildren[i], nodes, nodeIds);
}

void ModelImporterAssimp::loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, Node>& nodes, const AnimationAssimpNodes::NodeLoaderCallback& callback) const
{
    for(uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* animation = scene->mAnimations[i];
        animates.push_back(animation->mName.C_Str(), sp<AnimationAssimpNodes>::make(tps, animation, scene->mRootNode, globalTransformation, nodes, callback));
    }
}

void ModelImporterAssimp::loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, Node>& nodes, const AnimationAssimpNodes::NodeLoaderCallback& callback, String name, String alias) const
{
    for(uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* animate = scene->mAnimations[i];
        if(name == animate->mName.C_Str())
        {
            sp<Animation> animation = sp<AnimationAssimpNodes>::make(tps, animate, scene->mRootNode, globalTransformation, nodes, callback);
            if(alias)
            {
                if(animates.has(name))
                    name = std::move(alias);
                else
                    animates.push_back(std::move(alias), animation);
            }
            animates.push_back(std::move(name), std::move(animation));
            break;
        }
    }
}

Model ModelImporterAssimp::import(const Manifest& manifest, MaterialBundle& materialBundle)
{
    Assimp::Importer importer;
    return loadModel(loadScene(importer, manifest.src()), materialBundle, manifest);
}

const aiScene* ModelImporterAssimp::loadScene(Assimp::Importer& importer, const String& src, bool checkMeshes) const
{
    importer.SetIOHandler(new ArkIOSystem);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    uint32_t flags = static_cast<uint32_t>(aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes | aiProcess_LimitBoneWeights);
    const aiScene* scene = importer.ReadFile(src.c_str(), flags);
    CHECK(scene, "Loading \"%s\" failed", src.c_str());
    CHECK(!checkMeshes || scene->mNumMeshes > 0, "This scene(%s) has no meshes, maybe it's a scene contains animation-only informations. You should attach this one to its parent as an animation node.", src.c_str());
    return scene;
}

Mesh ModelImporterAssimp::loadMesh(const aiScene* scene, const aiMesh* mesh, MaterialBundle& materialBundle, element_index_t vertexBase, NodeTable& boneMapping, const std::vector<sp<Material>>& materials) const
{
    std::vector<element_index_t> indices = loadIndices(mesh, vertexBase);
    std::vector<V3> vertices(mesh->mNumVertices);
    sp<Array<Mesh::UV>> uvs = sp<Array<Mesh::UV>::Allocated>::make(mesh->mNumVertices);
    sp<Array<V3>> normals = mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>::null();
    sp<Array<Mesh::Tangent>> tangents = mesh->HasTangentsAndBitangents() ? sp<Array<Mesh::Tangent>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::Tangent>::Allocated>::null();
    sp<Array<Mesh::BoneInfo>> bones = mesh->HasBones() ? sp<Array<Mesh::BoneInfo>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::BoneInfo>::Allocated>::null();

    V3* vert = vertices.data() - 1;
    V3* norm = normals ? normals->buf() - 1 : nullptr;
    Mesh::Tangent* t = tangents ? tangents->buf() - 1 : nullptr;
    Mesh::UV* u = uvs->buf() - 1;

    DASSERT(mesh->mMaterialIndex < scene->mNumMaterials);
    const sp<Material>& material = materials[mesh->mMaterialIndex];
    const Rect uvBounds = materialBundle.getMaterialUV(material->name());
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

    return Mesh(mesh->mName.C_Str(), std::move(indices), std::move(vertices), std::move(uvs), std::move(normals), std::move(tangents), std::move(bones), material);
}

NodeTable ModelImporterAssimp::loadNodes(const aiNode* node, Model& model) const
{
    NodeTable nodes;
    std::unordered_map<uint32_t, uint32_t> nodeIds;
    loadNodeHierarchy(node, nodes, nodeIds);

    for(const auto& i : nodeIds)
        model.meshes().at(i.first)->setNodeId(sp<Integer::Const>::make(i.second));

    return nodes;
}

Model ModelImporterAssimp::loadModel(const aiScene* scene, MaterialBundle& materialBundle, const Manifest& manifest) const
{
    std::vector<sp<Mesh>> meshes;
    element_index_t vertexBase = 0;

    NodeTable bones;
    V3 aabbMin(std::numeric_limits<float>::max()), aabbMax(std::numeric_limits<float>::min());
    std::vector<sp<Material>> materials(scene->mNumMaterials);

    for(uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* am = scene->mMaterials[i];
        String mName = am->GetName().C_Str();
        sp<Material>& material = materials[i];
        if(!(material = materialBundle.getMaterial(mName)))
        {
            aiColor4D ac;
            material = sp<Material>::make(i, std::move(mName));
            if(aiGetMaterialColor(am, AI_MATKEY_COLOR_DIFFUSE, &ac) == aiReturn_SUCCESS)
                material->baseColor()->setColor(sp<Vec4::Const>::make(V4(ac.r, ac.g, ac.b, ac.a)));
        }
    }

    for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        meshes.push_back(sp<Mesh>::make(loadMesh(scene, mesh, materialBundle, vertexBase, bones, materials)));

        aabbMin = V3(std::min(mesh->mAABB.mMin.x, aabbMin.x()), std::min(mesh->mAABB.mMin.y, aabbMin.y()), std::min(mesh->mAABB.mMin.z, aabbMin.y()));
        aabbMax = V3(std::max(mesh->mAABB.mMax.x, aabbMax.x()), std::max(mesh->mAABB.mMax.y, aabbMax.y()), std::max(mesh->mAABB.mMax.z, aabbMax.y()));

        vertexBase += static_cast<element_index_t>(meshes.back()->vertexLength());
    }

    std::vector<document> animateManifests = manifest.descriptor() ? manifest.descriptor()->children("animate") : std::vector<document>();
    const bool hasAnimation = scene->HasAnimations() || animateManifests.size() > 0;
    aiMatrix4x4 globalAnimationTransform;
//    int32_t upAxis = -1;
//    if(scene->mMetaData->Get("UpAxis", upAxis) && false)
//    {
//        int32_t upAxisSign = 1;
//        scene->mMetaData->Get("UpAxisSign", upAxisSign);
//        DWARN(upAxis != 0, "X up axis does not supported");
//        if(upAxis == 1)
//        {
//            if(hasAnimation)
//                aiMatrix4x4::RotationX(-upAxisSign * Math::PI_2, globalAnimationTransform);
//            else
//            {
//                for(Mesh& i : meshes)
//                    yUp2zUp(i, upAxisSign > 0);
//                bounds = yUp2zUp(bounds, upAxisSign > 0);
//                bounds = V3(std::abs(bounds.x()), std::abs(bounds.y()), std::abs(bounds.z()));
//            }
//        }
//    }

    Model model(std::move(materials), std::move(meshes), Metrics(aabbMin, aabbMax, aabbMin, aabbMax));
    if(hasAnimation)
    {
        bool noBones = bones.nodes().size() == 0;
        Table<String, sp<Animation>> animates;
        AnimationAssimpNodes::NodeLoaderCallback callback = noBones ? callbackNodeAnimation : callbackBoneAnimation;
        NodeTable nodes = noBones ? loadNodes(scene->mRootNode, model) : std::move(bones);
        float defaultTps = manifest.getAttribute<float>("tps", 24.0f);
        loadAnimates(defaultTps, animates, scene, globalAnimationTransform, nodes.nodes(), callback);
        for(const auto& i : animateManifests)
        {
            Assimp::Importer importer;
            const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
            String name = Documents::getAttribute(i, Constants::Attributes::NAME);
            String alias = Documents::getAttribute(i, "alias");
            float tps = Documents::getAttribute<float>(i, "tps", defaultTps);
            const aiScene* animateScene = loadScene(importer, src, false);
            if(name)
                loadAnimates(tps, animates, animateScene, globalAnimationTransform, nodes.nodes(), callback, std::move(name), std::move(alias));
            else
                loadAnimates(tps, animates, animateScene, globalAnimationTransform, nodes.nodes(), callback);
        }

        model.setAnimations(std::move(animates));
        model.setNodeNames(nodes.nodes().keys());
    }

    return model;
}

void ModelImporterAssimp::loadBones(const aiMesh* mesh, NodeTable& boneMapping, Array<Mesh::BoneInfo>& bones) const
{
    Mesh::BoneInfo* bonesBuf = bones.buf();
    memset(bonesBuf, 0, bones.size());

    for(uint32_t i = 0; i < mesh->mNumBones; i++)
    {
        const String name(mesh->mBones[i]->mName.data);
        uint32_t index = boneMapping.ensureNode(name, mesh->mBones[i]->mOffsetMatrix)._id;

        for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
        {
            uint32_t vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
            Mesh::BoneInfo& boneInfo = bonesBuf[vertexID];
            boneInfo.add(index, mesh->mBones[i]->mWeights[j].mWeight);
        }
    }
}

//void ModelImporterAssimp::yUp2zUp(const Mesh& mesh, bool upSign)
//{
//    for(V3& v : *mesh.vertices())
//        v = yUp2zUp(v, upSign);
//    for(V3& n : *mesh.normals())
//        n = yUp2zUp(n, upSign);
//    for(Mesh::Tangent& t : *mesh.tangents())
//    {
//        t._tangent = yUp2zUp(t._tangent, upSign);
//        t._bitangent = yUp2zUp(t._bitangent, upSign);
//    }
//}

//V3 ModelImporterAssimp::yUp2zUp(const V3& p, bool upSign)
//{
//    return V3(p.x(), upSign ? -p.z() : p.z(), upSign ? p.y() : -p.y());
//}

aiMatrix4x4 ModelImporterAssimp::callbackNodeAnimation(const Node& /*node*/, const aiMatrix4x4& transform)
{
    return transform;
}

aiMatrix4x4 ModelImporterAssimp::callbackBoneAnimation(const Node& node, const aiMatrix4x4& transform)
{
    return transform * node._offset;
}

sp<ModelLoader::Importer> ModelImporterAssimp::BUILDER::build(const Scope& /*args*/)
{
    return sp<ModelImporterAssimp>::make();
}

}
}
}
