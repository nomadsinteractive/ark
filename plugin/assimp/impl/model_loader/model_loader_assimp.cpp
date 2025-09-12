#include "plugin/assimp/impl/model_loader/model_loader_assimp.h"

#include "core/base/manifest.h"
#include "core/impl/readable/bytearray_readable.h"
#include "core/inf/array.h"
#include "core/inf/loader.h"
#include "core/util/documents.h"
#include "core/util/log.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/material.h"
#include "graphics/components/size.h"

#include "renderer/base/animation.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/material_bundle.h"
#include "renderer/base/model.h"
#include "renderer/base/node.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "assimp/base/node_table.h"
#include "assimp/impl/io/ark_io_system.h"
#include "assimp/util/animate_util.h"


namespace ark::plugin::assimp {

namespace {

void loadNodeHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, AnimationNode>& nodes, const ModelImporterAssimp::NodeLoaderCallback& callback)
{
    const String nodeName(node->mName.data);
    const aiNodeAnim* pNodeAnim = AnimateUtil::findNodeAnim(animation, nodeName);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    if(pNodeAnim)
    {
        aiMatrix4x4 matScale = AnimateUtil::interpolateScale(tick, pNodeAnim);
        aiMatrix4x4 matRotation = AnimateUtil::interpolateRotation(tick, pNodeAnim);
        aiMatrix4x4 matTranslation = AnimateUtil::interpolateTranslation(tick, pNodeAnim);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    const aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;

    auto iter = nodes.find(nodeName);
    if (iter != nodes.end())
        iter->second._intermediate = callback(iter->second, globalTransformation);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        loadNodeHierarchy(tick, node->mChildren[i], animation, globalTransformation, nodes, callback);
}

void loadHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& transform, Table<String, AnimationNode>& nodes, const ModelImporterAssimp::NodeLoaderCallback& callback, Vector<M4>& output)
{
    loadNodeHierarchy(tick, node, animation, aiMatrix4x4(), nodes, callback);
    for(AnimationNode& i : nodes.values())
    {
        const aiMatrix4x4 finalMatrix = transform * i._intermediate;
        output.push_back(M4(finalMatrix).transpose());
    }
}

sp<Animation> makeAnimation(String name, float tps, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, Table<String, AnimationNode>& nodes, const ModelImporterAssimp::NodeLoaderCallback& callback)
{
    const float duration = static_cast<float>(animation->mDuration);
    const float tpsDefault = animation->mTicksPerSecond != 0 ? static_cast<float>(animation->mTicksPerSecond) : tps;
    const uint32_t durationInTicks = static_cast<uint32_t>(duration * tps / tpsDefault);

    Table<String, uint32_t> nodeIds;
    Vector<AnimationFrame> animationFrames;

    aiMatrix4x4 globalInversedTransform = rootNode->mTransformation * globalTransform;
    globalInversedTransform.Inverse();

    const float step = duration / static_cast<float>(durationInTicks);
    for(uint32_t i = 0; i < durationInTicks; ++i)
    {
        AnimationFrame frame;
        loadHierarchy(static_cast<float>(i) * step, rootNode, animation, globalInversedTransform, nodes, callback, frame);
        animationFrames.push_back(std::move(frame));
    }

    uint32_t index = 0;
    for(const auto& iter : nodes)
        nodeIds.push_back(iter.first, index++);

    return sp<Animation>::make(std::move(name), durationInTicks, std::move(nodeIds), std::move(animationFrames));
}


aiMatrix4x4 callbackNodeAnimation(const AnimationNode& /*node*/, const aiMatrix4x4& transform)
{
    return transform;
}

aiMatrix4x4 callbackBoneAnimation(const AnimationNode& node, const aiMatrix4x4& transform)
{
    return transform * node._offset;
}

}

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
    _textures.push_back(resourceLoaderContext.renderController()->createTexture2d(std::move(bitmap)));
}

Vector<sp<Material>> ModelImporterAssimp::loadMaterials(const aiScene* scene, MaterialBundle& materialBundle) const
{
    Vector<sp<Material>> materials(scene->mNumMaterials);

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
                material->baseColor()->setColor(sp<Vec4>::make<Vec4::Const>(V4(ac.r, ac.g, ac.b, ac.a)));
        }
    }

    return materials;
}

Vector<element_index_t> ModelImporterAssimp::loadIndices(const aiMesh* mesh, element_index_t vertexBase) const
{
    Vector<element_index_t> s(mesh->mNumFaces * 3);
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
        const AnimationNode& n = nodes.ensureNode(nodeName, node->mTransformation);
        for(uint32_t i = 0; i < node->mNumMeshes; ++i)
            nodeIds[node->mMeshes[i]] = n._id;
    }

    for(uint32_t i = 0; i < node->mNumChildren; ++i)
        loadNodeHierarchy(node->mChildren[i], nodes, nodeIds);
}

void ModelImporterAssimp::loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback) const
{
    for(uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* animation = scene->mAnimations[i];
        animates.push_back(animation->mName.C_Str(), makeAnimation(animation->mName.C_Str(), tps, animation, scene->mRootNode, globalTransformation, nodes, callback));
    }
}

void ModelImporterAssimp::loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback, String name, String alias) const
{
    for(uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* animate = scene->mAnimations[i];
        if(name == animate->mName.C_Str())
        {
            sp<Animation> animation = makeAnimation(name, tps, animate, scene->mRootNode, globalTransformation, nodes, callback);
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
    uint32_t flags = static_cast<uint32_t>(aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_LimitBoneWeights | aiProcess_OptimizeMeshes);
    if(Ark::instance().renderController()->renderEngine()->isLHS())
        flags |= aiProcess_FlipWindingOrder;
    LOGD("Loading scene %s", src.c_str());
    const aiScene* scene = importer.ReadFile(src.c_str(), flags);
    CHECK(scene, "Loading \"%s\" failed", src.c_str());
    CHECK(!checkMeshes || scene->mNumMeshes > 0, "This scene(%s) has no meshes, maybe it's a scene contains animation-only informations. You should attach this one to its parent as an animation node.", src.c_str());
    return scene;
}

Mesh ModelImporterAssimp::loadMesh(const aiScene* scene, const aiMesh* mesh, MaterialBundle& materialBundle, uint32_t meshId, element_index_t vertexBase, NodeTable& boneMapping, const Vector<sp<Material>>& materials) const
{
    Vector<element_index_t> indices = loadIndices(mesh, vertexBase);
    Vector<V3> vertices(mesh->mNumVertices);
    sp<Array<Mesh::UV>> uvs = sp<Array<Mesh::UV>::Allocated>::make(mesh->mNumVertices);
    sp<Array<V3>> normals = mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>();
    sp<Array<Mesh::Tangent>> tangents = mesh->HasTangentsAndBitangents() ? sp<Array<Mesh::Tangent>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::Tangent>::Allocated>();
    sp<Array<Mesh::BoneInfo>> bones = mesh->HasBones() ? sp<Array<Mesh::BoneInfo>::Allocated>::make(mesh->mNumVertices) : sp<Array<Mesh::BoneInfo>::Allocated>();

    V3* vert = vertices.data() - 1;
    V3* norm = normals ? normals->buf() - 1 : nullptr;
    Mesh::Tangent* t = tangents ? tangents->buf() - 1 : nullptr;
    Mesh::UV* u = uvs->buf() - 1;

    DASSERT(mesh->mMaterialIndex < scene->mNumMaterials);
    const sp<Material>& material = materials[mesh->mMaterialIndex];
    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        *(++vert) = V3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(norm)
            *(++norm) = V3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if(tangents)
            *(++t) = {V3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z), V3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z)};
        *(++u) = mesh->mTextureCoords[0] ? Mesh::UV{static_cast<uint16_t>(mesh->mTextureCoords[0][i].x * 0xffff), static_cast<uint16_t>(mesh->mTextureCoords[0][i].y * 0xffff)}
                                           : Mesh::UV{0, 0};
    }
    if(mesh->HasBones())
        loadBones(mesh, boneMapping, bones);

    return Mesh(meshId, mesh->mName.C_Str(), std::move(indices), std::move(vertices), std::move(uvs), std::move(normals), std::move(tangents), std::move(bones), material);
}

NodeTable ModelImporterAssimp::loadNodes(const aiNode* node, Model& model) const
{
    NodeTable nodes;
    std::unordered_map<uint32_t, uint32_t> nodeIds;
    loadNodeHierarchy(node, nodes, nodeIds);
    return nodes;
}

Model ModelImporterAssimp::loadModel(const aiScene* scene, MaterialBundle& materialBundle, const Manifest& manifest) const
{
    NodeTable bones;
    Vector<sp<Mesh>> meshes;
    element_index_t vertexBase = 0;
    Vector<sp<Material>> materials = loadMaterials(scene, materialBundle);

    for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        sp<Mesh> m = sp<Mesh>::make(loadMesh(scene, mesh, materialBundle, i, vertexBase, bones, materials));
        meshes.push_back(std::move(m));
        vertexBase += static_cast<element_index_t>(meshes.back()->vertexCount());
    }

    sp<Node> rootNode = loadNodeHierarchy({}, scene->mRootNode, meshes);

    Vector<document> animateManifests = manifest.descriptor() ? manifest.descriptor()->children("animate") : Vector<document>();
    const bool hasAnimation = scene->HasAnimations() || animateManifests.size() > 0;

    Model model(std::move(materials), std::move(meshes), std::move(rootNode));
    if(hasAnimation)
    {
        aiMatrix4x4 globalAnimationTransform;
        const bool noBones = bones.nodes().empty();
        Table<String, sp<Animation>> animates;
        NodeLoaderCallback callback = noBones ? callbackNodeAnimation : callbackBoneAnimation;
        NodeTable animationNodes = noBones ? loadNodes(scene->mRootNode, model) : std::move(bones);
        float defaultTps = manifest.getAttribute<float>("tps", 24.0f);
        loadAnimates(defaultTps, animates, scene, globalAnimationTransform, animationNodes.nodes(), callback);
        for(const auto& i : animateManifests)
        {
            Assimp::Importer importer;
            const String& src = Documents::ensureAttribute(i, constants::SRC);
            String name = Documents::getAttribute(i, constants::NAME);
            String alias = Documents::getAttribute(i, "alias");
            float tps = Documents::getAttribute<float>(i, "tps", defaultTps);
            const aiScene* animateScene = loadScene(importer, src, false);
            if(name)
                loadAnimates(tps, animates, animateScene, globalAnimationTransform, animationNodes.nodes(), callback, std::move(name), std::move(alias));
            else
                loadAnimates(tps, animates, animateScene, globalAnimationTransform, animationNodes.nodes(), callback);
        }

        model.setAnimations(std::move(animates));
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

sp<ModelLoader::Importer> ModelImporterAssimp::BUILDER::build(const Scope& /*args*/)
{
    return sp<ModelImporterAssimp>::make();
}

sp<Node> ModelImporterAssimp::loadNodeHierarchy(WeakPtr<Node> parentNode, const aiNode* node, const Vector<sp<Mesh>>& meshes) const
{
    sp<Node> n = sp<Node>::make(std::move(parentNode), node->mName.C_Str(), M4(node->mTransformation).transpose());
    for(uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        DASSERT(node->mMeshes[i] < meshes.size());
        n->addMesh(meshes[node->mMeshes[i]]);
    }
    n->calculateLocalAABB();
    for(uint32_t i = 0; i < node->mNumChildren; ++i)
        n->childNodes().push_back(loadNodeHierarchy(n, node->mChildren[i], meshes));
    return n;
}

}
