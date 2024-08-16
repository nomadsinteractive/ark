#include "renderer/opengl/base/gl_pipeline.h"

#include "core/inf/uploader.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/uniform.h"
#include "renderer/inf/resource.h"
#include "renderer/util/render_util.h"

#include "renderer/opengl/util/gl_debug.h"
#include "renderer/opengl/util/gl_util.h"

#include "platform/platform.h"

namespace ark::opengl {

namespace {

class GLScissor {
public:
    GLScissor(const Optional<Rect>& scissor)
        : _enabled(static_cast<bool>(scissor)) {
        if(_enabled) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(static_cast<GLint>(scissor->left()), static_cast<GLint>(scissor->top()), static_cast<GLsizei>(scissor->width()), static_cast<GLsizei>(scissor->height()));
        }
    }
    ~GLScissor() {
        if(_enabled)
            glDisable(GL_SCISSOR_TEST);
    }

private:
    bool _enabled;
};

class GLCullFaceTest final : public Snippet::DrawEvents {
public:
    GLCullFaceTest(const PipelineDescriptor::TraitCullFaceTest& trait)
        : _enabled(trait._enabled), _front_face(trait._front_face == PipelineDescriptor::FRONT_FACE_DEFAULT ? GL_ZERO : (trait._front_face == PipelineDescriptor::FRONT_FACE_CLOCK_WISE ? GL_CW : GL_CCW)), _pre_front_face(GL_ZERO) {
    }

    void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        if(_enabled) {
            if(_front_face != GL_ZERO) {
                if(_pre_front_face == GL_ZERO)
                    glGetIntegerv(GL_FRONT_FACE, reinterpret_cast<GLint*>(&_pre_front_face));
                glFrontFace(_front_face);
            }
        }
        else
            glDisable(GL_CULL_FACE);
    }

    void postDraw(GraphicsContext& /*graphicsContext*/) override {
        if(_enabled) {
            if(_front_face != GL_ZERO)
                glFrontFace(_pre_front_face);
        }
        else
            glEnable(GL_CULL_FACE);
    }

private:
    bool _enabled;
    GLenum _front_face;
    GLenum _pre_front_face;
};


class GLDepthTest final : public Snippet::DrawEvents {
public:
    GLDepthTest(const PipelineDescriptor::TraitDepthTest& trait)
        : _func(GLUtil::toCompareFunc(trait._func)), _pre_func(GL_ZERO), _enabled(trait._enabled), _read_only(!trait._write_enabled) {
    }

    void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        if(_enabled) {
            if(_func != GL_ZERO) {
                if(_pre_func == GL_ZERO)
                    glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&_pre_func));
                glDepthFunc(_func);
            }
            if(_read_only)
                glDepthMask(GL_FALSE);
        }
        else
            glDisable(GL_DEPTH_TEST);
    }

    void postDraw(GraphicsContext& /*graphicsContext*/) override {
        if(!_enabled)
            glEnable(GL_DEPTH_TEST);
        if(_func != GL_ZERO)
            glDepthFunc(_pre_func);
        if(_read_only)
            glDepthMask(GL_TRUE);
    }

private:
    GLenum _func;
    GLenum _pre_func;
    bool _enabled;
    bool _read_only;
};


class GLStencilTest final : public Snippet::DrawEvents {
public:
    GLStencilTest(std::vector<sp<Snippet::DrawEvents>> delegate)
        : _delegate(std::move(delegate)) {
    }

    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        glEnable(GL_STENCIL_TEST);
        for(const sp<Snippet::DrawEvents>& i : _delegate)
            i->preDraw(graphicsContext, context);
    }

    virtual void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glStencilMask(0);
        glDisable(GL_STENCIL_TEST);
    }

private:
    std::vector<sp<Snippet::DrawEvents>> _delegate;
};

class GLStencilTestSeparate final : public Snippet::DrawEvents {
public:
    GLStencilTestSeparate(const PipelineDescriptor::TraitStencilTestSeparate& conf)
        : _face(GLUtil::toFrontFaceType(conf._type)), _mask(conf._mask), _func(GLUtil::toCompareFunc(conf._func)), _compare_mask(conf._compare_mask),
          _ref(conf._ref), _op(GLUtil::toStencilFunc(conf._op)), _op_dfail(GLUtil::toStencilFunc(conf._op_dfail)), _op_dpass(GLUtil::toStencilFunc(conf._op_dpass)) {
    }

    void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        glStencilMaskSeparate(_face, _mask);
        glStencilFuncSeparate(_face, _func, _ref, _compare_mask);
        glStencilOpSeparate(_face, _op, _op_dfail, _op_dpass);
    }

private:
    GLenum _face;
    GLuint _mask;
    GLenum _func;
    GLuint _compare_mask;
    GLint _ref;

    GLenum _op, _op_dfail, _op_dpass;
};

class GLTraitBlend final : public Snippet::DrawEvents {
public:
    GLTraitBlend(const PipelineDescriptor::TraitBlend& conf)
        : _src_rgb_factor(GLUtil::toBlendFactor(conf._src_rgb_factor)), _dest_rgb_factor(GLUtil::toBlendFactor(conf._dst_rgb_factor)),
          _src_alpha_factor(GLUtil::toBlendFactor(conf._src_alpha_factor)), _dest_alpha_factor(GLUtil::toBlendFactor(conf._dst_alpha_factor)) {
    }

    void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        glGetIntegerv(GL_BLEND_SRC_RGB, &_src_rgb_factor_default);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &_src_alpha_factor_default);
        glGetIntegerv(GL_BLEND_DST_RGB, &_dest_rgb_factor_default);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &_dest_alpha_factor_default);
        glBlendFuncSeparate(_src_rgb_factor != std::numeric_limits<GLenum>::max() ? _src_rgb_factor : _src_rgb_factor_default, _dest_rgb_factor != std::numeric_limits<GLenum>::max() ? _dest_rgb_factor : _dest_rgb_factor_default,
                            _src_alpha_factor != std::numeric_limits<GLenum>::max() ? _src_alpha_factor : _src_alpha_factor_default, _dest_alpha_factor != std::numeric_limits<GLenum>::max() ? _dest_alpha_factor : _dest_alpha_factor_default);
    }

    void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glBlendFuncSeparate(_src_rgb_factor_default, _dest_rgb_factor_default, _src_alpha_factor_default, _dest_alpha_factor_default);
    }

private:
    GLenum _src_rgb_factor;
    GLenum _dest_rgb_factor;
    GLenum _src_alpha_factor;
    GLenum _dest_alpha_factor;

    GLenum _src_rgb_factor_default;
    GLenum _dest_rgb_factor_default;
    GLenum _src_alpha_factor_default;
    GLenum _dest_alpha_factor_default;
};

class GLDrawArrays final : public PipelineDrawCommand {
public:
    GLDrawArrays(GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
        DASSERT(drawingContext._draw_count);
        glDrawArrays(_mode, param._start * sizeof(element_index_t), static_cast<GLsizei>(drawingContext._draw_count));
    }

private:
    GLenum _mode;
};

class GLDrawElements final : public PipelineDrawCommand {
public:
    GLDrawElements(GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
        DASSERT(drawingContext._draw_count);
        glDrawElements(_mode, static_cast<GLsizei>(drawingContext._draw_count), GLIndexType, reinterpret_cast<GLvoid*>(param._start * sizeof(element_index_t)));
    }

private:
    GLenum _mode;
};

class GLDrawElementsInstanced final : public PipelineDrawCommand {
public:
    GLDrawElementsInstanced(GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const DrawingParams::DrawElementsInstanced& param = drawingContext._parameters.drawElementsInstanced();
        DASSERT(param._count);
        DASSERT(drawingContext._draw_count);
        for(const auto& [i, j] : param._divided_buffer_snapshots)
        {
            j.upload(graphicsContext);
            DCHECK(j.id(), "Invaild Divided Buffer: %d", i);
        }
        glDrawElementsInstanced(_mode, static_cast<GLsizei>(param._count), GLIndexType, nullptr, drawingContext._draw_count);
    }
private:
    GLenum _mode;
};

struct GLMultiDrawElementsIndirect final : PipelineDrawCommand {
    GLMultiDrawElementsIndirect(GLenum mode)
        : _mode(mode)
    {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();

        for(const auto& [k, v] : param._divided_buffer_snapshots)
        {
            v.upload(graphicsContext);
            DCHECK(v.id(), "Invaild Divided Buffer Buffer: %d", k);
        }
        param._indirect_cmds.upload(graphicsContext);

        const volatile GLBufferBinder binder(GL_DRAW_INDIRECT_BUFFER, static_cast<GLuint>(param._indirect_cmds.id()));
#ifndef ANDROID
        glMultiDrawElementsIndirect(_mode, GLIndexType, nullptr, static_cast<GLsizei>(param._indirect_cmd_count), sizeof(DrawingParams::DrawElementsIndirectCommand));
#else
        for(uint32_t i = 0; i < param._indirect_cmd_count; ++i)
            glDrawElementsIndirect(_mode, GLIndexType, reinterpret_cast<const void *>(i * sizeof(DrawingParams::DrawElementsIndirectCommand)));
#endif
    }

    GLenum _mode;
};

}

GLPipeline::GLPipeline(const sp<Recycler>& recycler, uint32_t version, std::map<PipelineInput::ShaderStage, String> shaders, const PipelineDescriptor& bindings)
    : _stub(sp<Stub>::make()), _recycler(recycler), _version(version), _shaders(std::move(shaders)), _pipeline_operation(makePipelineOperation(bindings))
{
    for(const auto& i : bindings.parameters()._traits)
    {
        if(i.first == PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST)
            _draw_decorators.push_back(sp<GLCullFaceTest>::make(i.second._configure._cull_face_test));
        else if(i.first == PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)
            _draw_decorators.push_back(sp<GLDepthTest>::make(i.second._configure._depth_test));
        else if(i.first == PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST)
        {
            std::vector<sp<Snippet::DrawEvents>> delegate;
            const PipelineDescriptor::TraitStencilTest& test = i.second._configure._stencil_test;
            if(test._front._type == PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT && test._front._type == test._back._type)
                delegate.push_back(sp<GLStencilTestSeparate>::make(test._front));
            else
            {
                if(test._front._type == PipelineDescriptor::FRONT_FACE_TYPE_FRONT)
                    delegate.push_back(sp<GLStencilTestSeparate>::make(test._front));
                if(test._back._type == PipelineDescriptor::FRONT_FACE_TYPE_BACK)
                    delegate.push_back(sp<GLStencilTestSeparate>::make(test._back));
            }
            DASSERT(delegate.size() > 0);
            _draw_decorators.push_back(sp<GLStencilTest>::make(std::move(delegate)));
        }
        else if(i.first == PipelineDescriptor::TRAIT_TYPE_BLEND)
            _draw_decorators.push_back(sp<GLTraitBlend>::make(i.second._configure._blend));
    }
}

GLPipeline::~GLPipeline()
{
    _recycler->recycle(*this);
}

uint64_t GLPipeline::id()
{
    return _stub->_id;
}

void GLPipeline::upload(GraphicsContext& graphicsContext)
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
    CHECK(linkstatus, "Program link failed: %s", getInformationLog(id).c_str());
}

ResourceRecycleFunc GLPipeline::recycle()
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
    for(const sp<Snippet::DrawEvents>& i : _draw_decorators)
        i->preDraw(graphicsContext, drawingContext);
    _pipeline_operation->draw(graphicsContext, drawingContext);
    for(const sp<Snippet::DrawEvents>& i : _draw_decorators)
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
    for(const auto& [i, j] : divisors)
    {
        if(!j.id())
            j.upload(graphicsContext);

        const volatile GLBufferBinder binder(GL_ARRAY_BUFFER, static_cast<GLuint>(j.id()));
        bindBuffer(graphicsContext, input, i);
    }
}

void GLPipeline::activeTexture(const Texture& texture, const String& name, uint32_t binding)
{
    _stub->activeTexture(texture, name, binding);
}

const GLPipeline::GLUniform& GLPipeline::getUniform(const String& name)
{
    return _stub->getUniform(name);
}

void GLPipeline::bindBuffer(GraphicsContext& /*graphicsContext*/, const PipelineInput& input, uint32_t divisor)
{
    const PipelineInput::StreamLayout& stream = input.getStreamLayout(divisor);
    for(const auto& i : stream.attributes().values())
    {
        const GLPipeline::GLAttribute& glAttribute = _stub->getAttribute(i.name());
        glAttribute.bind(i, stream.stride());
    }
}

sp<GLPipeline::PipelineOperation> GLPipeline::makePipelineOperation(const PipelineDescriptor& bindings) const
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

    const sp<ShaderPool>& shaders = graphicsContext.attachments().ensure<ShaderPool>();
    const auto iter = (*shaders)[type].find(source);
    if(iter != (*shaders)[type].end())
    {
        const sp<GLPipeline::Stage> shader = iter->second.lock();
        if(shader)
            return shader;
    }

    const sp<Stage> shader = sp<Stage>::make(graphicsContext.renderController()->recycler(), version, type, source);
    (*shaders)[type][source] = shader;
    return shader;
}

GLPipeline::GLAttribute::GLAttribute(GLint location)
    : _location(location)
{
}

void GLPipeline::GLAttribute::bind(const Attribute& attribute, GLsizei stride) const
{
    CHECK_WARN(_location >= 0, "Attribute \"%s\" location: %d", attribute.name().c_str(), _location);
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
    else
    {
        FATAL("Unknow attribute \"%s %s\"", attribute.type(), attribute.name().c_str());
    }
}

void GLPipeline::GLAttribute::setVertexPointer(const Attribute& attribute, GLuint location, GLsizei stride, uint32_t length, uint32_t offset) const
{
    static const GLenum glTypes[Attribute::TYPE_COUNT] = {GL_BYTE, GL_FLOAT, GL_INT, GL_SHORT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT};
    glEnableVertexAttribArray(location);
    if(attribute.type() == Attribute::TYPE_FLOAT || attribute.normalized())
        glVertexAttribPointer(location, length, glTypes[attribute.type()], attribute.normalized() ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
    else
        glVertexAttribIPointer(location, length, glTypes[attribute.type()], stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));

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

void GLPipeline::GLUniform::setUniform4fv(GLsizei count, const GLfloat* value) const
{
    glUniform4fv(_location, count, value);
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
        FATAL("%s\n\n%s", logs.data(), sb.str().c_str());
        return 0;
    }
    return id;
}

GLPipeline::PipelineOperationDraw::PipelineOperationDraw(sp<Stub> stub, const PipelineDescriptor& bindings)
    : _stub(std::move(stub)), _scissor(bindings.scissor()), _renderer(makeBakedRenderer(bindings))
{
}

void GLPipeline::PipelineOperationDraw::bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    _stub->bind(graphicsContext, drawingContext._pipeline_snapshot);
}

void GLPipeline::PipelineOperationDraw::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    const volatile GLScissor scissor(drawingContext._scissor ? drawingContext._scissor : _scissor);

    for(const auto& [i, j] : drawingContext._pipeline_snapshot._ssbos)
        _ssbo_binders.emplace_back(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(i), static_cast<GLuint>(j.id()));

    _renderer->draw(graphicsContext, drawingContext);

    _ssbo_binders.clear();
}

void GLPipeline::PipelineOperationDraw::compute(GraphicsContext& /*graphicsContext*/, const ComputeContext& /*computeContext*/)
{
    DFATAL("This is a drawing pipeline, not compute");
}

sp<PipelineDrawCommand> GLPipeline::PipelineOperationDraw::makeBakedRenderer(const PipelineDescriptor& bindings) const
{
    GLenum mode = GLUtil::toEnum(bindings.mode());
    switch(bindings.drawProcedure())
    {
    case Enum::DRAW_PROCEDURE_DRAW_ARRAYS:
        return sp<GLDrawArrays>::make(mode);
    case Enum::DRAW_PROCEDURE_DRAW_ELEMENTS:
        return sp<GLDrawElements>::make(mode);
    case Enum::DRAW_PROCEDURE_DRAW_INSTANCED:
        return sp<GLDrawElementsInstanced>::make(mode);
    case Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT:
        return sp<GLMultiDrawElementsIndirect>::make(mode);
    }
    DFATAL("Not render procedure creator for %d", bindings.drawProcedure());
    return nullptr;
}

GLPipeline::Stub::Stub()
    : _id(0), _rebind_needed(true)
{
}

void GLPipeline::Stub::bind(GraphicsContext& /*graphicsContext*/, const PipelineSnapshot& pipelineContext)
{
    glUseProgram(_id);

    const PipelineBindings& pipelineBindings = pipelineContext._bindings;
    bindUBOSnapshots(pipelineContext._ubos, pipelineBindings.pipelineInput());

    uint32_t binding = 0;
    for(const auto& [k, v] : pipelineBindings.pipelineDescriptor()->samplers())
    {
        CHECK_WARN(v, "Pipeline has unbound sampler \"%s\"", k.c_str());
        if(v)
            activeTexture(v, k, binding);
        ++ binding;
    }

    const std::vector<sp<Texture>>& images = pipelineBindings.pipelineDescriptor()->images();
    for(size_t i = 0; i < images.size(); ++i)
        if(const sp<Texture>& image = images.at(i))
            bindImage(image, static_cast<uint32_t>(i));
}

void GLPipeline::Stub::bindUBO(const RenderLayerSnapshot::UBOSnapshot& uboSnapshot, const sp<PipelineInput::UBO>& ubo)
{
    const std::vector<sp<Uniform>>& uniforms = ubo->uniforms().values();
    for(size_t i = 0; i < uniforms.size(); ++i)
    {
        if(uboSnapshot._dirty_flags.buf()[i] || _rebind_needed)
        {
            const sp<Uniform>& uniform = uniforms.at(i);
            const uint8_t* buf = uboSnapshot._buffer.buf();
            const auto& [offset, size] = ubo->slots().at(i);
            bindUniform(buf + offset, size, uniform);
        }
    }
}

void GLPipeline::Stub::bindUniform(const uint8_t* ptr, uint32_t size, const Uniform& uniform)
{
    const GLPipeline::GLUniform& glUniform = getUniform(uniform.name());
    const float* ptrf = reinterpret_cast<const float*>(ptr);
    switch(uniform.type()) {
    case Uniform::TYPE_I1:
        DCHECK(size == 4, "Wrong uniform1i size: %d", size);
        glUniform.setUniform1i(*reinterpret_cast<const int32_t*>(ptr));
        break;
    case Uniform::TYPE_F1:
        DCHECK(size == 4, "Wrong uniform1f size: %d", size);
        glUniform.setUniform1f(ptrf[0]);
        break;
    case Uniform::TYPE_F2:
        DCHECK(size == 8, "Wrong uniform2f size: %d", size);
        glUniform.setUniform2f(ptrf[0], ptrf[1]);
        break;
    case Uniform::TYPE_F3:
        DCHECK(size >= 12, "Wrong uniform3f size: %d", size);
        glUniform.setUniform3f(ptrf[0], ptrf[1], ptrf[2]);
        break;
    case Uniform::TYPE_F4:
        DCHECK(size == 16, "Wrong uniform4f size: %d", size);
        glUniform.setUniform4f(ptrf[0], ptrf[1], ptrf[2], ptrf[3]);
        break;
    case Uniform::TYPE_F4V:
        DCHECK(size % 16 == 0, "Wrong uniform4fv size: %d", size);
        glUniform.setUniform4fv(size / 16, ptrf);
        break;
    case Uniform::TYPE_MAT4:
    case Uniform::TYPE_MAT4V:
        DCHECK(size % 64 == 0, "Wrong mat4fv size: %d", size);
        glUniform.setUniformMatrix4fv(size / 64, GL_FALSE, ptrf);
        break;
    case Uniform::TYPE_IMAGE2D:
    case Uniform::TYPE_UIMAGE2D:
    case Uniform::TYPE_IIMAGE2D:
        break;
    default:
        DFATAL("Unimplemented");
    }
}

void GLPipeline::Stub::bindImage(const Texture& texture, uint32_t name)
{
    const char uniformName[16] = {'u', '_', 'I', 'm', 'a', 'g', 'e', static_cast<char>('0' + name)};
    const GLPipeline::GLUniform& uImage = getUniform(uniformName);
    const Texture::Format textureFormat = texture.parameters()->_format;
    const uint32_t channelSize = RenderUtil::getChannelSize(textureFormat);
    const uint32_t componentSize = RenderUtil::getComponentSize(textureFormat);
    const GLenum format = GLUtil::getTextureInternalFormat(Texture::USAGE_GENERAL, texture.parameters()->_format, channelSize, componentSize);
    uImage.setUniform1i(static_cast<GLint>(name));
    glBindImageTexture(name, static_cast<GLuint>(texture.delegate()->id()), 0, GL_FALSE, 0, GL_READ_WRITE, format);
}

void GLPipeline::Stub::activeTexture(const Texture& texture, const String& name, uint32_t binding)
{
    static GLenum glTargets[Texture::TYPE_COUNT] = {GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP};
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + binding));
    glBindTexture(glTargets[texture.type()], static_cast<GLuint>(texture.delegate()->id()));

    const GLPipeline::GLUniform& uTexture = getUniform(name);
    uTexture.setUniform1i(static_cast<GLint>(binding));
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
    CHECK_WARN(location != -1, "Undefined uniform \"%s\". It might be optimized out, or something goes wrong.", name.c_str());
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

void GLPipeline::Stub::bindUBOSnapshots(const std::vector<RenderLayerSnapshot::UBOSnapshot>& uboSnapshots, const PipelineInput& pipelineInput)
{
    DCHECK(uboSnapshots.size() == pipelineInput.ubos().size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), pipelineInput.ubos().size());

    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderLayerSnapshot::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        const sp<PipelineInput::UBO>& ubo = pipelineInput.ubos().at(i);
        bindUBO(uboSnapshot, ubo);
    }
    _rebind_needed = false;
}

GLPipeline::PipelineOperationCompute::PipelineOperationCompute(sp<GLPipeline::Stub> stub)
    : _stub(std::move(stub))
{
}

void GLPipeline::PipelineOperationCompute::bind(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*computeContext*/)
{
}

void GLPipeline::PipelineOperationCompute::draw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*computeContext*/)
{
}

void GLPipeline::PipelineOperationCompute::compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    _stub->bind(graphicsContext, computeContext._pipeline_context);

    for(const auto& [i, j] : computeContext._pipeline_context._ssbos)
        _ssbo_binders.emplace_back(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(i), static_cast<GLuint>(j.id()));

    glDispatchCompute(computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));

    _ssbo_binders.clear();
}

GLPipeline::GLBufferBaseBinder::GLBufferBaseBinder(GLenum target, GLuint base, GLuint buffer)
    : _target(target), _base(base), _buffer(buffer)
{
    glBindBufferBase(_target, _base, _buffer);
}

GLPipeline::GLBufferBaseBinder::GLBufferBaseBinder(GLBufferBaseBinder&& other)
    : _target(other._target), _base(other._base), _buffer(other._buffer)
{
    other._buffer = 0;
}

GLPipeline::GLBufferBaseBinder::~GLBufferBaseBinder()
{
    if(_buffer)
        glBindBufferBase(_target, _base, 0);
}

}
