#include "plugin/assimp/impl/model_loader/model_loader_assimp.h"

#include "core/impl/readable/bytearray_readable.h"
#include "core/inf/array.h"
#include "core/inf/loader.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/base/size.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/uniform.h"

#include "assimp/impl/io/ark_io_system.h"
#include "assimp/impl/vertices/vertices_assimp.h"


namespace ark {
namespace plugin {
namespace assimp {

ModelLoaderAssimp::ModelLoaderAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest)
    : ModelLoader(RenderModel::RENDER_MODE_TRIANGLES), _importer(sp<Assimp::Importer>::make()), _model_matrics(sp<Array<M4>::Allocated>::make(32))
{
    _importer->SetIOHandler(new ArkIOSystem());

    for(const document& i : manifest->children())
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        const aiScene* scene = _importer->ReadFile(src.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);
        _models[type] = loadModel(scene->mMeshes[0]);

        for(uint32_t i = 0; i < scene->mNumTextures; ++i)
            loadSceneTexture(resourceLoaderContext, scene->mTextures[i]);
    }

    _importer->FreeScene();
}

void ModelLoaderAssimp::initialize(ShaderBindings& shaderBindings)
{
    const sp<Uniform> uniform = shaderBindings.pipelineInput()->getUniform("u_ModelMatrix");
    uniform->setFlatable(sp<Flatable::Array<M4>>::make(_model_matrics));
    for(const sp<Texture>& i : _textures)
        shaderBindings.pipelineBindings()->bindSampler(i);
}

//sp<ShaderBindings> ModelLoaderAssimp::makeShaderBindings(const Shader& shader)
//{
//    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLES, shader.renderController()->makeVertexBuffer(), shader.renderController()->makeIndexBuffer());

//    const sp<Uniform> uniform = bindings->pipelineInput()->getUniform("u_ModelMatrix");
//    uniform->setFlatable(sp<Flatable::Array<M4>>::make(_model_matrics));
//    for(const sp<Texture>& i : _textures)
//        bindings->pipelineBindings()->bindSampler(i);
//    return bindings;
//}

void ModelLoaderAssimp::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    DCHECK(snapshot._items.size() <= _model_matrics->length(), "Cannot support more than %d renderobjects", _model_matrics->length());
    for(size_t i = 0; i < snapshot._items.size(); ++i)
    {
        const Renderable::Snapshot& ro = snapshot._items.at(i);
        const Transform::Snapshot& transform = ro._transform;
        _model_matrics->buf()[i] = MatrixUtil::mul(MatrixUtil::translate(M4::identity(), ro._position), transform.toMatrix());
    }
}

Model ModelLoaderAssimp::load(int32_t type)
{
    const auto iter = _models.find(type);
    DCHECK(iter != _models.end(), "Model not found, type: %d", type);
    return iter->second;
}

//void ModelLoaderAssimp::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
//{
//    const Buffer& ibo = buf.shaderBindings()->indexBuffer();

//    if(snapshot._flag != RenderLayer::SNAPSHOT_FLAG_STATIC_REUSE)
//    {
//        DWARN(snapshot._flag != RenderLayer::SNAPSHOT_FLAG_DYNAMIC_UPDATE, "Dynamic layer for 3D models is inefficiency");
//        std::vector<sp<Array<element_index_t>>> indices;
//        for(size_t i = 0; i < snapshot._items.size(); ++i)
//        {
//            const Renderable::Snapshot& ro = snapshot._items.at(i);
//            const auto iter = _models.find(ro._type);
//            DCHECK(iter != _models.end(), "Model %d does not exist", ro._type);
//            const sp<Model>& model = iter->second;

//            element_index_t base = static_cast<element_index_t>(buf.vertices().length());
//            if(base != 0)
//            {
//                size_t length = model->indices()->length();
//                const sp<Array<element_index_t>> index = sp<Array<element_index_t>::Allocated>::make(length);
//                element_index_t* src = model->indices()->buf();
//                element_index_t* dst = index->buf();
//                for(size_t j = 0; j < length; ++j, ++src, ++dst)
//                    *dst = base + *src;
//                indices.push_back(index);
//            }
//            else
//                indices.push_back(model->indices());

//            compose(model, static_cast<int32_t>(i), buf);
//        }
//        buf.setIndices(ibo.snapshot(sp<Uploader::ArrayList<element_index_t>>::make(std::move(indices))));
//    }
//    else
//        buf.setIndices(ibo.snapshot());
//}

void ModelLoaderAssimp::loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex)
{
    const bitmap bitmap = loadBitmap(resourceLoaderContext.images(), tex);
    _textures.push_back(resourceLoaderContext.renderController()->createTexture2D(sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height())), sp<Texture::UploaderBitmap>::make(bitmap)));
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

Model ModelLoaderAssimp::loadModel(const aiMesh* mesh) const
{
    sp<Array<element_index_t>> indices = loadIndices(mesh);

    sp<Array<V3>> vertices = sp<Array<V3>::Allocated>::make(mesh->mNumVertices);
    sp<Array<VerticesAssimp::UV>> uvs = sp<Array<VerticesAssimp::UV>::Allocated>::make(mesh->mNumVertices);
    sp<Array<V3>> normals = mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>::null();
    sp<Array<VerticesAssimp::Tangent>> tangents = mesh->HasTangentsAndBitangents() ? sp<Array<VerticesAssimp::Tangent>::Allocated>::make(mesh->mNumVertices) : sp<Array<VerticesAssimp::Tangent>::Allocated>::null();

    V3* vert = vertices->buf() - 1;
    V3* norm = normals ? normals->buf() - 1 : nullptr;
    VerticesAssimp::Tangent* t = tangents ? tangents->buf() - 1 : nullptr;
    VerticesAssimp::UV* u = uvs->buf() - 1;
    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        *(++vert) = V3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(norm)
            *(++norm) = V3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if(tangents)
            *(++t) = VerticesAssimp::Tangent(V3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z), V3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
        *(++u) = VerticesAssimp::UV(static_cast<uint16_t>(mesh->mTextureCoords[0][i].x * 0xffff), static_cast<uint16_t>(mesh->mTextureCoords[0][i].y * 0xffff));
    }

    return Model(indices, sp<VerticesAssimp>::make(std::move(vertices), std::move(uvs), std::move(normals), std::move(tangents)));
}

array<element_index_t> ModelLoaderAssimp::loadIndices(const aiMesh* mesh) const
{
    const array<element_index_t> s = sp<Array<element_index_t>::Allocated>::make(mesh->mNumFaces * 3);
    element_index_t* buf = s->buf();
    for(uint32_t i = 0; i < mesh->mNumFaces; i ++)
    {
        buf[2] = static_cast<element_index_t>(mesh->mFaces[i].mIndices[0]);
        buf[1] = static_cast<element_index_t>(mesh->mFaces[i].mIndices[1]);
        buf[0] = static_cast<element_index_t>(mesh->mFaces[i].mIndices[2]);
        buf += 3;
    }
    return s;
}

void ModelLoaderAssimp::compose(const Model& model, int32_t modelId, DrawingBuffer& buf) const
{
    DFATAL("Unimplemented");
//    V3* vertices = model.vertices()->buf();
//    V3* normals = model.normals() ? model.normals()->buf() : nullptr;
//    Model::Tangents* tangents = model.tangents() ? model.tangents()->buf() : nullptr;
//    Model::UV* uvs = model.uvs()->buf();

//    size_t length = model.vertices()->length();
//    for(size_t i = 0; i < length; ++i)
//    {
//        buf.nextVertex();
//        buf.writePosition(*(vertices++));
//        if(normals)
//            buf.writeNormal(*(normals++));
//        if(tangents)
//        {
//            buf.writeTangent(tangents->_tangent);
//            ++tangents;
//        }
//        buf.writeModelId(modelId);
//        buf.writeTexCoordinate(uvs->_u, uvs->_v);
//        ++uvs;
//    }
}

ModelLoaderAssimp::BUILDER::BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _manifest(manifest) {
}

sp<ModelLoader> ModelLoaderAssimp::BUILDER::build(const Scope& /*args*/)
{
    return sp<ModelLoaderAssimp>::make(_resource_loader_context, _manifest);
}

}
}
}
