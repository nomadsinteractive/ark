#include "renderer/opengl/base/gl_pipeline.h"

#include "core/inf/flatable.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/uniform.h"
#include "renderer/inf/resource.h"
#include "renderer/util/render_util.h"

#include "renderer/opengl/util/gl_util.h"

#include "platform/platform.h"

namespace ark {
namespace opengl {

namespace {

class GLScissor {
public:
    GLScissor(const Rect& rect, bool enabled)
        : _enabled(enabled) {
        if(_enabled) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(static_cast<GLint>(rect.left()), static_cast<GLint>(rect.top()), static_cast<GLsizei>(rect.width()), static_cast<GLsizei>(rect.height()));
        }
    }
    ~GLScissor() {
        if(_enabled)
            glDisable(GL_SCISSOR_TEST);
    }

private:
    bool _enabled;
};

class GLCullFace {
public:
    GLCullFace(bool enabled)
        : _enabled(enabled) {
        if(_enabled)
            glEnable(GL_CULL_FACE);
    }
    ~GLCullFace() {
        if(_enabled)
            glDisable(GL_CULL_FACE);
    }

private:
    bool _enabled;
};

class GLBufferBinder {
public:
    GLBufferBinder(GLenum target, GLuint buffer)
        : _target(target), _buffer(buffer) {
        glBindBuffer(_target, _buffer);
    }
    ~GLBufferBinder() {
        glBindBuffer(_target, 0);
    }

private:
    GLenum _target;
    GLuint _buffer;
};

class GLBufferBaseBinder {
public:
    GLBufferBaseBinder(GLenum target, GLuint base, GLuint buffer)
        : _target(target), _base(base), _buffer(buffer) {
        glBindBufferBase(_target, _base, _buffer);
    }
    ~GLBufferBaseBinder() {
        glBindBufferBase(_target, _base, 0);
    }

private:
    GLenum _target;
    GLuint _base;
    GLuint _buffer;
};

class GLStencilTest : public Snippet::DrawEvents {
public:
    GLStencilTest(const document& manifest)
        : _func(GLUtil::getEnum(manifest, "func")), _ref(Documents::ensureAttribute<int32_t>(manifest, "ref")), _mask(Documents::getAttribute<uint32_t>(manifest, "mask", 0xff)),
          _op(GLUtil::getEnum(manifest, "op", GL_KEEP)), _op_dfail(GLUtil::getEnum(manifest, "op-dfail", GL_KEEP)), _op_dpass(GLUtil::getEnum(manifest, "op-dpass", GL_KEEP)) {
    }

    virtual void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(_func, _ref, _mask);
        glStencilOp(_op, _op_dfail, _op_dpass);
    }

    virtual void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glStencilMask(0);
        glDisable(GL_STENCIL_TEST);
    }

private:
    GLenum _func;
    GLint _ref;
    GLuint _mask;

    GLenum _op, _op_dfail, _op_dpass;
};

}

GLPipeline::GLPipeline(const sp<Recycler>& recycler, uint32_t version, std::map<PipelineInput::ShaderStage, String> shaders, const PipelineBindings& bindings)
    : _stub(sp<Stub>::make()), _recycler(recycler), _version(version), _shaders(std::move(shaders)), _pipeline_operation(makePipelineOperation(bindings))
{
    for(const auto& i : bindings.parameters()._tests)
        if(i.first == PipelineBindings::FRAGMENT_TEST_STENCIL)
            _draw_tests.push_back(sp<GLStencilTest>::make(i.second));
}

GLPipeline::~GLPipeline()
{
    _recycler->recycle(*this);
}

uint64_t GLPipeline::id()
{
    return _stub->_id;
}

void GLPipeline::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    GLuint id = glCreateProgram();

    _stub->_rebind_needed = true;
    _stub->_id = id;

    std::map<PipelineInput::ShaderStage, sp<GLPipeline::Stage>> compiledShaders;

    for(const auto& i : _shaders)
    {
        sp<GLPipeline::Stage>& shader = compiledShaders[i.first];
        shader = makeShader(graphicsContext, _version, GLUtil::toShaderType(i.first), i.second);
        glAttachShader(id, shader->id());
    }

    glLinkProgram(id);

    for(const auto& i : compiledShaders)
        glDetachShader(id, i.second->id());

    GLint linkstatus = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &linkstatus);
    DCHECK(linkstatus, "Program link failed: %s", getInformationLog(id).c_str());
    LOGD("GLProgram[%d]:", id);
}

Resource::RecycleFunc GLPipeline::recycle()
{
    uint32_t id = _stub->_id;
    _stub->_id = 0;

    _stub->_attributes.clear();
    _stub->_uniforms.clear();

    return [id](GraphicsContext&) {
        LOGD("glDeleteProgram(%d)", id);
        glDeleteProgram(id);
    };
}

void GLPipeline::bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    _pipeline_operation->bind(graphicsContext, drawingContext);
}

void GLPipeline::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    for(const sp<Snippet::DrawEvents>& i : _draw_tests)
        i->preDraw(graphicsContext, drawingContext);
    _pipeline_operation->draw(graphicsContext, drawingContext);
    for(const sp<Snippet::DrawEvents>& i : _draw_tests)
        i->postDraw(graphicsContext);
}

void GLPipeline::compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    _pipeline_operation->compute(graphicsContext, computeContext);
}

void GLPipeline::bindBuffer(GraphicsContext& graphicsContext, const PipelineInput& input, const std::map<uint32_t, Buffer>& divisors)
{
    DCHECK(id(), "GLProgram unprepared");
    bindBuffer(graphicsContext, input, 0);
    for(const auto& i : divisors)
    {
        if(!i.second.id())
            i.second.upload(graphicsContext);

        const GLBufferBinder binder(GL_ARRAY_BUFFER, static_cast<GLuint>(i.second.id()));
        bindBuffer(graphicsContext, input, i.first);
    }
}

void GLPipeline::activeTexture(const Texture& texture, uint32_t name)
{
    _stub->activeTexture(texture, name);
}

const GLPipeline::GLUniform& GLPipeline::getUniform(const String& name)
{
    return _stub->getUniform(name);
}

void GLPipeline::bindBuffer(GraphicsContext& /*graphicsContext*/, const PipelineInput& input, uint32_t divisor)
{
    const PipelineInput::Stream& stream = input.getStream(divisor);
    for(const auto& i : stream.attributes().values())
    {
        const GLPipeline::GLAttribute& glAttribute = _stub->getAttribute(i.name());
        glAttribute.bind(i, stream.stride());
    }
}

sp<GLPipeline::PipelineOperation> GLPipeline::makePipelineOperation(const PipelineBindings& bindings) const
{
    for(const auto& i : _shaders)
        if(i.first == PipelineInput::SHADER_STAGE_COMPUTE)
        {
            DCHECK(_shaders.size() == 1, "Compute shader is an exclusive stage");
            return sp<PipelineOperationCompute>::make(_stub);
        }

    return sp<PipelineOperationDraw>::make(_stub, bindings);
}

String GLPipeline::getInformationLog(GLuint id) const
{
    GLint length = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

    size_t len = static_cast<size_t>(length);
    std::vector<GLchar> infos(len + 1);
    glGetProgramInfoLog(id, length, &length, infos.data());
    infos.back() = 0;
    return infos.data();
}

sp<GLPipeline::Stage> GLPipeline::makeShader(GraphicsContext& graphicsContext, uint32_t version, GLenum type, const String& source) const
{
    typedef std::unordered_map<GLenum, std::map<String, WeakPtr<Stage>>> ShaderPool;

    ShaderPool& shaders = *graphicsContext.attachments().ensure<ShaderPool>();
    const auto iter = shaders[type].find(source);
    if(iter != shaders[type].end())
    {
        const sp<GLPipeline::Stage> shader = iter->second.lock();
        if(shader)
            return shader;
    }

    const sp<Stage> shader = sp<Stage>::make(graphicsContext.renderController()->recycler(), version, type, source);
    shaders[type][source] = shader;
    return shader;
}

GLPipeline::GLAttribute::GLAttribute(GLint location)
    : _location(location)
{
}

void GLPipeline::GLAttribute::bind(const Attribute& attribute, GLsizei stride) const
{
    DWARN(_location >= 0, "Attribute \"%s\" location: %d", attribute.name().c_str(), _location);
    if(attribute.length() <= 4)
        setVertexPointer(attribute, _location, stride, attribute.length(), attribute.offset());
    else if(attribute.length() == 16)
    {
        uint32_t offset = attribute.size() / 4;
        for(int32_t i = 0; i < 4; i++)
            setVertexPointer(attribute, _location + i, stride, 4, attribute.offset() + offset * i);
    }
    else if(attribute.length() == 9)
    {
        uint32_t offset = attribute.size() / 3;
        for(int32_t i = 0; i < 3; i++)
            setVertexPointer(attribute, _location + i, stride, 3, attribute.offset() + offset * i);
    }
}

void GLPipeline::GLAttribute::setVertexPointer(const Attribute& attribute, GLuint location, GLsizei stride, uint32_t length, uint32_t offset) const
{
    static const GLenum glTypes[Attribute::TYPE_COUNT] = {GL_BYTE, GL_FLOAT, GL_INT, GL_SHORT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT};

    glEnableVertexAttribArray(location);
    if(attribute.type() == Attribute::TYPE_FLOAT || attribute.normalized())
        glVertexAttribPointer(location, length, glTypes[attribute.type()], attribute.normalized() ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<void*>(offset));
    else
        glVertexAttribIPointer(location, length, glTypes[attribute.type()], stride, reinterpret_cast<void*>(offset));

    if(attribute.divisor())
        glVertexAttribDivisor(location, attribute.divisor());
}

GLPipeline::GLUniform::GLUniform(GLint location)
    : _location(location)
{
}

GLPipeline::GLUniform::operator bool() const
{
    return _location != -1;
}

void GLPipeline::GLUniform::setUniform1i(GLint x) const
{
    glUniform1i(_location, x);
}

void GLPipeline::GLUniform::setUniform1f(GLfloat x) const
{
    glUniform1f(_location, x);
}

void GLPipeline::GLUniform::setUniform2f(GLfloat x, GLfloat y) const
{
    glUniform2f(_location, x, y);
}

void GLPipeline::GLUniform::setUniform3f(GLfloat x, GLfloat y, GLfloat z) const
{
    glUniform3f(_location, x, y, z);
}

void GLPipeline::GLUniform::setUniform4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
{
    glUniform4f(_location, r, g, b, a);
}

void GLPipeline::GLUniform::setUniform4fv(GLsizei count, GLfloat* value) const
{
    glUniform4fv(_location, count, value);
}

void GLPipeline::GLUniform::setUniformColor4f(const Color& color) const
{
    glUniform4f(_location, color.r(), color.g(), color.b(), color.a());
}

void GLPipeline::GLUniform::setUniformMatrix4fv(GLsizei count, GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(_location, count, transpose, value);
}

GLPipeline::Stage::Stage(const sp<Recycler>& recycler, uint32_t version, GLenum type, const String& source)
    : _recycler(recycler), _id(compile(version, type, source))
{
}

GLPipeline::Stage::~Stage()
{
    uint32_t id = _id;
    _recycler->recycle([id](GraphicsContext&) {
        LOGD("glDeleteShader(%d)", id);
        glDeleteShader(id);
    });
}

uint32_t GLPipeline::Stage::id()
{
    return _id;
}

GLuint GLPipeline::Stage::compile(uint32_t version, GLenum type, const String& source)
{
    const String versionSrc = source.startsWith("#version ") ? "" : Platform::glShaderVersionDeclaration(version);
    GLuint id = glCreateShader(type);
    const GLchar* src[16] = {versionSrc.c_str()};
    uint32_t slen = Platform::glPreprocessShader(source, &src[1], 15);
    glShaderSource(id, slen + 1, src, nullptr);
    glCompileShader(id);
    GLint compiled;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GLint length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        size_t len = static_cast<size_t>(length);
        std::vector<GLchar> logs(len + 1);
        glGetShaderInfoLog(id, length, &length, logs.data());
        logs.back() = 0;
        StringBuffer sb;
        for(uint32_t i = 0; i <= slen; i++)
            sb << src[i] << '\n';
        DFATAL("%s\n\n%s", logs.data(), sb.str().c_str());
        return 0;
    }
    return id;
}

GLPipeline::GLDrawArrays::GLDrawArrays(GLenum mode)
    : _mode(mode)
{
}

void GLPipeline::GLDrawArrays::draw(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext)
{
    const DrawingContext::ParamDrawElements& param = drawingContext._parameters._draw_elements;
    DASSERT(param._count);
    glDrawArrays(_mode, param._start * sizeof(element_index_t), static_cast<GLsizei>(param._count));
}

GLPipeline::GLDrawElements::GLDrawElements(GLenum mode)
    : _mode(mode)
{
}

void GLPipeline::GLDrawElements::draw(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext)
{
    const DrawingContext::ParamDrawElements& param = drawingContext._parameters._draw_elements;
    DASSERT(param._count);
    glDrawElements(_mode, static_cast<GLsizei>(param._count), GLIndexType, reinterpret_cast<GLvoid*>(param._start * sizeof(element_index_t)));
}

GLPipeline::GLDrawElementsInstanced::GLDrawElementsInstanced(GLenum mode)
    : _mode(mode)
{
}

void GLPipeline::GLDrawElementsInstanced::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    const DrawingContext::ParamDrawElementsInstanced& param = drawingContext._parameters._draw_elements_instanced;
    DASSERT(param.isActive());
    DASSERT(param._count);
    for(const auto& i : param._instanced_array_snapshots)
    {
        i.second.upload(graphicsContext);
        DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
    }
    glDrawElementsInstanced(_mode, static_cast<GLsizei>(param._count), GLIndexType, nullptr, param._instance_count);
}

GLPipeline::GLMultiDrawElementsIndirect::GLMultiDrawElementsIndirect(GLenum mode)
    : _mode(mode)
{
}

void GLPipeline::GLMultiDrawElementsIndirect::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    const DrawingContext::ParamDrawMultiElementsIndirect& param = drawingContext._parameters._draw_multi_elements_indirect;
    DASSERT(param.isActive());

    for(const auto& i : param._instanced_array_snapshots)
    {
        i.second.upload(graphicsContext);
        DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
    }
    param._indirect_cmds.upload(graphicsContext);

    const GLBufferBinder binder(GL_DRAW_INDIRECT_BUFFER, static_cast<GLuint>(param._indirect_cmds.id()));
#ifndef ANDROID
    glMultiDrawElementsIndirect(_mode, GLIndexType, nullptr, static_cast<GLsizei>(param._draw_count), sizeof(DrawingContext::DrawElementsIndirectCommand));
#else
    for(uint32_t i = 0; i < param._draw_count; ++i)
        glDrawElementsIndirect(_mode, GLIndexType, reinterpret_cast<const void *>(i * sizeof(DrawingContext::DrawElementsIndirectCommand)));
#endif
}

GLPipeline::PipelineOperationDraw::PipelineOperationDraw(const sp<Stub>& stub, const PipelineBindings& bindings)
    : _stub(stub), _cull_face(bindings.getFlag(PipelineBindings::FLAG_CULL_MODE_BITMASK) != PipelineBindings::FLAG_CULL_MODE_NONE),
      _scissor(bindings.scissor()), _scissor_enabled(RenderUtil::isScissorEnabled(_scissor)), _renderer(makeBakedRenderer(bindings))
{
}

void GLPipeline::PipelineOperationDraw::bind(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext)
{
    glUseProgram(_stub->_id);
    _stub->bindUBOSnapshots(drawingContext._ubos, drawingContext._shader_bindings->pipelineInput());

    const std::vector<sp<Texture>>& samplers = drawingContext._shader_bindings->samplers();
    for(size_t i = 0; i < samplers.size(); ++i)
    {
        const sp<Texture>& sampler = samplers.at(i);
        DWARN(sampler, "Pipeline has unbound sampler at: %d", i);
        if(sampler)
            _stub->activeTexture(sampler, static_cast<uint32_t>(i));
    }
}

void GLPipeline::PipelineOperationDraw::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    bool contextScissorEnabled = RenderUtil::isScissorEnabled(drawingContext._scissor);

    const GLCullFace cullFace(_cull_face);
    const GLScissor scissor(contextScissorEnabled ? drawingContext._scissor : _scissor, contextScissorEnabled || _scissor_enabled);

    std::vector<GLBufferBaseBinder> binders;
    for(const Buffer::Snapshot& i : drawingContext._ssbos)
        binders.emplace_back(GL_SHADER_STORAGE_BUFFER, 0, i.id());

    _renderer->draw(graphicsContext, drawingContext);
}

void GLPipeline::PipelineOperationDraw::compute(GraphicsContext& /*graphicsContext*/, const ComputeContext& /*computeContext*/)
{
    DFATAL("This is a drawing pipeline, not compute");
}

sp<GLPipeline::BakedRenderer> GLPipeline::PipelineOperationDraw::makeBakedRenderer(const PipelineBindings& bindings) const
{
    GLenum mode = GLUtil::toEnum(bindings.mode());
    switch(bindings.renderProcedure())
    {
    case PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS:
        return sp<GLDrawArrays>::make(mode);
    case PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS:
        return sp<GLDrawElements>::make(mode);
    case PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS_INSTANCED:
        DASSERT(bindings.hasDivisors());
        return sp<GLDrawElementsInstanced>::make(mode);
    case PipelineBindings::RENDER_PROCEDURE_DRAW_MULTI_ELEMENTS_INDIRECT:
        return sp<GLMultiDrawElementsIndirect>::make(mode);
    }
    DFATAL("Not render procedure creator for %d", bindings.renderProcedure());
    return nullptr;
}

GLPipeline::Stub::Stub()
    : _id(0), _rebind_needed(true)
{
}

void GLPipeline::Stub::bindUBO(const RenderLayer::UBOSnapshot& uboSnapshot, const sp<PipelineInput::UBO>& ubo)
{
    const std::vector<sp<Uniform>>& uniforms = ubo->uniforms().values();
    for(size_t i = 0; i < uniforms.size(); ++i)
    {
        if(uboSnapshot._dirty_flags.buf()[i] || _rebind_needed)
        {
            const sp<Uniform>& uniform = uniforms.at(i);
            uint8_t* buf = uboSnapshot._buffer.buf();
            const auto pair = ubo->slots().at(i);
            bindUniform(reinterpret_cast<float*>(buf + pair.first), pair.second, uniform);
        }
    }
}

void GLPipeline::Stub::bindUniform(float* buf, uint32_t size, const Uniform& uniform)
{
    const GLPipeline::GLUniform& glUniform = getUniform(uniform.name());
    switch(uniform.type()) {
    case Uniform::TYPE_I1:
        DCHECK(size == 4, "Wrong uniform1i size: %d", size);
        glUniform.setUniform1i(*reinterpret_cast<int32_t*>(buf));
        break;
    case Uniform::TYPE_F1:
        DCHECK(size == 4, "Wrong uniform1f size: %d", size);
        glUniform.setUniform1f(buf[0]);
        break;
    case Uniform::TYPE_F2:
        DCHECK(size == 8, "Wrong uniform2f size: %d", size);
        glUniform.setUniform2f(buf[0], buf[1]);
        break;
    case Uniform::TYPE_F3:
        DCHECK(size >= 12, "Wrong uniform3f size: %d", size);
        glUniform.setUniform3f(buf[0], buf[1], buf[2]);
        break;
    case Uniform::TYPE_F4:
        DCHECK(size == 16, "Wrong uniform4f size: %d", size);
        glUniform.setUniform4f(buf[0], buf[1], buf[2], buf[3]);
        break;
    case Uniform::TYPE_F4V:
        DCHECK(size % 16 == 0, "Wrong uniform4fv size: %d", size);
        glUniform.setUniform4fv(size / 16, buf);
        break;
    case Uniform::TYPE_MAT4:
    case Uniform::TYPE_MAT4V:
        DCHECK(size % 64 == 0, "Wrong mat4fv size: %d", size);
        glUniform.setUniformMatrix4fv(size / 64, GL_FALSE, buf);
        break;
    default:
        DFATAL("Unimplemented");
    }
}

void GLPipeline::Stub::activeTexture(const Texture& texture, uint32_t name)
{
    static GLenum glTargets[Texture::TYPE_COUNT] = {GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP};
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + name));
    glBindTexture(glTargets[texture.type()], static_cast<GLuint>(texture.delegate()->id()));

    char uniformName[16] = {'u', '_', 'T', 'e', 'x', 't', 'u', 'r', 'e', static_cast<char>('0' + name)};
    const GLPipeline::GLUniform& uTexture = getUniform(uniformName);
    uTexture.setUniform1i(static_cast<GLint>(name));
}

const GLPipeline::GLUniform& GLPipeline::Stub::getUniform(const String& name)
{
    const auto iter = _uniforms.find(name);
    if(iter != _uniforms.end())
        return iter->second;
    _uniforms[name] = getUniformLocation(name);
    return _uniforms[name];
}

GLint GLPipeline::Stub::getUniformLocation(const String& name)
{
    GLint location = glGetUniformLocation(_id, name.c_str());
    DWARN(location != -1, "Undefined uniform \"%s\". It might be optimized out, or something goes wrong.", name.c_str());
    return location;
}

const GLPipeline::GLAttribute& GLPipeline::Stub::getAttribute(const String& name)
{
    const auto iter = _attributes.find(name);
    if(iter != _attributes.end())
        return iter->second;
    _attributes[name] = getAttribLocation(name);
    return _attributes[name];
}

GLint GLPipeline::Stub::getAttribLocation(const String& name)
{
    return glGetAttribLocation(_id, name.c_str());
}

void GLPipeline::Stub::bindUBOSnapshots(const std::vector<RenderLayer::UBOSnapshot>& uboSnapshots, const PipelineInput& pipelineInput)
{
    DCHECK(uboSnapshots.size() == pipelineInput.ubos().size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), pipelineInput.ubos().size());

    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderLayer::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        const sp<PipelineInput::UBO>& ubo = pipelineInput.ubos().at(i);
        bindUBO(uboSnapshot, ubo);
    }
    _rebind_needed = false;
}

GLPipeline::PipelineOperationCompute::PipelineOperationCompute(const sp<GLPipeline::Stub>& stub)
    : _stub(stub)
{
}

void GLPipeline::PipelineOperationCompute::bind(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*computeContext*/)
{
}

void GLPipeline::PipelineOperationCompute::draw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*computeContext*/)
{
}

void GLPipeline::PipelineOperationCompute::compute(GraphicsContext& /*graphicsContext*/, const ComputeContext& computeContext)
{
    glUseProgram(_stub->_id);
    _stub->bindUBOSnapshots(computeContext._ubos, computeContext._shader_bindings->pipelineInput());

    std::vector<GLBufferBaseBinder> binders;
    for(const Buffer::Snapshot& i : computeContext._ssbo)
        binders.emplace_back(GL_SHADER_STORAGE_BUFFER, 0, i.id());

    glDispatchCompute(computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));
}

}
}
