#include "plugin/assimp/impl/render_model/render_model_assimp.h"

#include "core/impl/readable/bytearray_readable.h"
#include "core/inf/array.h"
#include "core/inf/loader.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/base/matrix.h"
#include "graphics/base/size.h"

#include "renderer/base/model.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/uniform.h"

#include "assimp/impl/io/ark_io_system.h"


namespace ark {
namespace plugin {
namespace assimp {

RenderModelAssimp::RenderModelAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest)
    : _importer(sp<Assimp::Importer>::make()), _model_matrics(sp<Array<Matrix>::Allocated>::make(32))
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

sp<ShaderBindings> RenderModelAssimp::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLES, shader.renderController()->makeVertexBuffer(), shader.renderController()->makeIndexBuffer());

    const sp<Uniform> uniform = bindings->pipelineInput()->getUniform("u_ModelMatrix");
    uniform->setFlatable(sp<Flatable::Array<Matrix>>::make(_model_matrics));
    for(const sp<Texture>& i : _textures)
        bindings->pipelineBindings()->bindSampler(i);
    return bindings;
}

void RenderModelAssimp::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    DCHECK(snapshot._items.size() <= _model_matrics->length(), "Cannot support more than %d renderobjects", _model_matrics->length());
    for(size_t i = 0; i < snapshot._items.size(); ++i)
    {
        const RenderObject::Snapshot& ro = snapshot._items.at(i);
        Transform::Snapshot transform = ro._transform;
        transform.pivot += ro._position;
        _model_matrics->buf()[i] = transform.toMatrix();
    }
}

void RenderModelAssimp::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    const Buffer& ibo = buf.shaderBindings()->indexBuffer();

    if(snapshot._flag != RenderLayer::SNAPSHOT_FLAG_STATIC_REUSE)
    {
        DWARN(snapshot._flag != RenderLayer::SNAPSHOT_FLAG_DYNAMIC, "Dynamic layer for 3D models is inefficiency");
        std::vector<sp<Array<element_index_t>>> indices;
        buf.vertices().setGrowCapacity(100000);
        for(size_t i = 0; i < snapshot._items.size(); ++i)
        {
            const RenderObject::Snapshot& ro = snapshot._items.at(i);
            const auto iter = _models.find(ro._type);
            DCHECK(iter != _models.end(), "Model %d does not exist", ro._type);
            const sp<Model>& model = iter->second;

            element_index_t base = static_cast<element_index_t>(buf.vertices().length());
            if(base != 0)
            {
                size_t length = model->indices()->length();
                const sp<Array<element_index_t>> index = sp<Array<element_index_t>::Allocated>::make(length);
                element_index_t* src = model->indices()->buf();
                element_index_t* dst = index->buf();
                for(size_t j = 0; j < length; ++j, ++src, ++dst)
                    *dst = base + *src;
                indices.push_back(index);
            }
            else
                indices.push_back(model->indices());

            compose(model, static_cast<int32_t>(i), buf);
        }
        buf.setIndices(ibo.snapshot(sp<Uploader::ArrayList<element_index_t>>::make(std::move(indices))));
    }
    else
        buf.setIndices(ibo.snapshot());
}

void RenderModelAssimp::load(DrawingBuffer& /*buf*/, const RenderObject::Snapshot& /*snapshot*/)
{
}

void RenderModelAssimp::loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex)
{
    const bitmap bitmap = loadBitmap(resourceLoaderContext.images(), tex);
    _textures.push_back(resourceLoaderContext.renderController()->createTexture2D(sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height())), sp<Texture::UploaderBitmap>::make(bitmap)));
}

bitmap RenderModelAssimp::loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const
{
    if(tex->mHeight == 0)
    {
        const sp<BitmapLoader>& bitmapLoader = imageResource->getLoader(tex->achFormatHint);
        return bitmapLoader->load(sp<BytearrayReadable>::make(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth)));
    }
    return bitmap::make(tex->mWidth, tex->mHeight, tex->mWidth * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(tex->pcData), tex->mWidth * tex->mHeight * 4));
}

sp<Model> RenderModelAssimp::loadModel(const aiMesh* mesh)
{
    sp<Array<element_index_t>> indices = loadIndices(mesh);

    sp<Array<V3>> vertices = sp<Array<V3>::Allocated>::make(mesh->mNumVertices);
    sp<Array<Model::UV>> uvs = sp<Array<Model::UV>::Allocated>::make(mesh->mNumVertices);
    sp<Array<V3>> normals = mesh->HasNormals() ? sp<Array<V3>::Allocated>::make(mesh->mNumVertices) : sp<Array<V3>::Allocated>::null();
    sp<Array<Model::Tangents>> tangents = mesh->HasTangentsAndBitangents() ? sp<Array<Model::Tangents>::Allocated>::make(mesh->mNumVertices) : sp<Array<Model::Tangents>::Allocated>::null();

    V3* vert = vertices->buf() - 1;
    V3* norm = normals ? normals->buf() - 1 : nullptr;
    Model::Tangents* t = tangents ? tangents->buf() - 1 : nullptr;
    Model::UV* u = uvs->buf() - 1;
    for(uint32_t i = 0; i < mesh->mNumVertices; i ++)
    {
        *(++vert) = V3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(norm)
            *(++norm) = V3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if(tangents)
            *(++t) = Model::Tangents(V3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z), V3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
        *(++u) = Model::UV(static_cast<uint16_t>(mesh->mTextureCoords[0][i].x * 0xffff), static_cast<uint16_t>(mesh->mTextureCoords[0][i].y * 0xffff));
    }

    return sp<Model>::make(std::move(indices), std::move(vertices), std::move(uvs), std::move(normals), std::move(tangents));
}

array<element_index_t> RenderModelAssimp::loadIndices(const aiMesh* mesh) const
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

void RenderModelAssimp::compose(const Model& model, int32_t modelId, DrawingBuffer& buf) const
{
    V3* vertices = model.vertices()->buf();
    V3* normals = model.normals() ? model.normals()->buf() : nullptr;
    Model::Tangents* tangents = model.tangents() ? model.tangents()->buf() : nullptr;
    Model::UV* uvs = model.uvs()->buf();

    size_t length = model.vertices()->length();
    for(size_t i = 0; i < length; ++i)
    {
        buf.nextVertex();
        buf.writePosition(*(vertices++));
        if(normals)
            buf.writeNormal(*(normals++));
        if(tangents)
        {
            buf.writeTangent(tangents->_tangent);
            ++tangents;
        }
        buf.writeModelId(modelId);
        buf.writeTexCoordinate(uvs->_u, uvs->_v);
        ++uvs;
    }
}

RenderModelAssimp::BUILDER::BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _manifest(manifest) {
}

sp<RenderModel> RenderModelAssimp::BUILDER::build(const Scope& /*args*/)
{
    return sp<RenderModelAssimp>::make(_resource_loader_context, _manifest);
}

}
}
}
