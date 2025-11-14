#include "opengl/base/gl_pipeline.h"

#include <ranges>

#include "core/types/weak_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/uniform.h"
#include "renderer/inf/draw_decorator.h"
#include "renderer/inf/resource.h"
#include "renderer/util/render_util.h"

#include "opengl/impl/es30/gl_resource/gl_vertex_array.h"
#include "opengl/util/gl_debug.h"
#include "opengl/util/gl_util.h"

#include "platform/platform.h"
#include "renderer/base/pipeline_descriptor.h"

namespace ark::plugin::opengl {

namespace {

void setVertexPointer(const Attribute& attribute, const GLuint location, const GLsizei stride, const uint32_t length, const uint32_t offset)
{
    constexpr GLenum glTypes[Attribute::TYPE_COUNT] = {GL_BYTE, GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_SHORT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT};
    glEnableVertexAttribArray(location);
    if(attribute.type() == Attribute::TYPE_FLOAT || attribute.normalized())
        GL_CHECK_ERROR(glVertexAttribPointer(location, length, glTypes[attribute.type()], attribute.normalized() ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset))));
    else
        GL_CHECK_ERROR(glVertexAttribIPointer(location, length, glTypes[attribute.type()], stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset))));

    if(attribute.divisor())
        GL_CHECK_ERROR(glVertexAttribDivisor(location, attribute.divisor()));
}

GLuint compileShader(const uint32_t version, const GLenum type, const String& source)
{
    const String versionSrc = source.startsWith("#version ") ? "" : Platform::glShaderVersionDeclaration(version);
    const GLuint id = glCreateShader(type);
    const GLchar* src[16] = {versionSrc.c_str()};
    const uint32_t slen = Platform::glPreprocessShader(source, &src[1], 15);
    glShaderSource(id, slen + 1, src, nullptr);
    glCompileShader(id);
    GLint compiled;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GLint length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        const size_t len = static_cast<size_t>(length);
        Vector<GLchar> logs(len + 1);
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

class GLAttribute {
public:
    GLAttribute(const GLint location = -1)
        : _location(location) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(GLAttribute);

    void bind(const Attribute& attribute, const GLsizei stride) const
    {
        CHECK_WARN_OR_RETURN(_location >= 0, "Attribute \"%s\" location: %d", attribute.name().c_str(), _location);
        if(attribute.length() <= 4)
            setVertexPointer(attribute, _location, stride, attribute.length(), attribute.offset());
        else if(attribute.length() == 16)
        {
            const uint32_t offset = attribute.size() / 4;
            for(int32_t i = 0; i < 4; i++)
                setVertexPointer(attribute, _location + i, stride, 4, attribute.offset() + offset * i);
        }
        else if(attribute.length() == 9)
        {
            const uint32_t offset = attribute.size() / 3;
            for(int32_t i = 0; i < 3; i++)
                setVertexPointer(attribute, _location + i, stride, 3, attribute.offset() + offset * i);
        }
        else
        {
            FATAL("Unknow attribute \"%s %s\"", attribute.type(), attribute.name().c_str());
        }
    }

private:
    GLint _location;
};

}

struct GLPipeline::Stub {
    Stub(const bool isComputePipeline)
        : _is_compute_pipeline(isComputePipeline), _id(0), _rebind_needed(true) {
    }

    void bind(GraphicsContext& /*graphicsContext*/, const PipelineContext& pipelineContext)
    {
        GL_CHECK_ERROR(glUseProgram(_id));

        const PipelineBindings& pipelineBindings = pipelineContext._bindings;
        bindUBOSnapshots(pipelineContext._buffer_snapshot->_ubos, pipelineBindings.pipelineLayout());

        uint32_t binding = 0;
        const Vector<String>& samplerNames = pipelineBindings.pipelineLayout()->samplers().keys();
        const auto& samplers = pipelineBindings.samplers();
        DASSERT(samplerNames.size() == samplers.size());
        for(size_t i = 0; i < samplerNames.size(); ++i)
        {
            const String& name = samplerNames.at(i);
            const sp<Texture>& texture = samplers.at(i)._texture;
            DASSERT(name == samplers.at(i)._name);
            CHECK_WARN(texture, "Pipeline has unbound sampler \"%s\"", name.c_str());
            if(texture)
                activeTexture(texture, name, binding);
            ++ binding;
        }

        const Vector<String>& imageNames = pipelineBindings.pipelineLayout()->images().keys();
        const Vector<PipelineDescriptor::BindedTexture>& images = pipelineBindings.images();
        for(size_t i = 0; i < images.size(); ++i)
            if(const sp<Texture>& image = images.at(i)._texture)
            {
                const String& name = imageNames.at(i);
                bindImage(image, name, static_cast<uint32_t>(i));
            }
    }

    void bindUBO(const RenderBufferSnapshot::UBOSnapshot& uboSnapshot, const PipelineLayout::UBO& ubo)
    {
        const Vector<sp<Uniform>>& uniforms = ubo.uniforms().values();
        for(size_t i = 0; i < uniforms.size(); ++i)
        {
            if(uboSnapshot._dirty_flags.buf()[i] || _rebind_needed)
            {
                const sp<Uniform>& uniform = uniforms.at(i);
                const uint8_t* buf = uboSnapshot._buffer.buf();
                const auto& [offset, size] = ubo.slots().at(i);
                bindUniform(buf + offset, size, uniform);
            }
        }
    }

    void bindUniform(const uint8_t* ptr, const uint32_t size, const Uniform& uniform)
    {
        const GLUniform& glUniform = getUniform(uniform.name());
        const float* ptrf = reinterpret_cast<const float*>(ptr);
        switch(uniform.type()) {
            case Uniform::TYPE_I1:
                DCHECK(size == 4, "Wrong uniform1i size: %d", size);
                glUniform.setUniform1i(*reinterpret_cast<const int32_t*>(ptr));
            break;
            case Uniform::TYPE_I2:
                DCHECK(size == 8, "Wrong uniform2i size: %d", size);
                glUniform.setUniform2i(*reinterpret_cast<const int32_t*>(ptr), *(reinterpret_cast<const int32_t*>(ptr) + 1));
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

    void bindImage(const Texture& texture, const String& name, const uint32_t binding)
    {
        const GLUniform& uImage = getUniform(name);
        const Texture::Format textureFormat = texture.parameters()->_format;
        const uint32_t channelSize = RenderUtil::getChannelSize(textureFormat);
        const uint32_t componentSize = RenderUtil::getComponentSize(textureFormat);
        const GLenum format = GLUtil::getTextureInternalFormat(Texture::USAGE_AUTO, texture.parameters()->_format, channelSize, componentSize);
        uImage.setUniform1i(static_cast<GLint>(binding));
        GL_CHECK_ERROR(glBindImageTexture(binding, static_cast<GLuint>(texture.delegate()->id()), 0, GL_FALSE, 0, GL_READ_WRITE, format));
    }

    void activeTexture(const Texture& texture, const String& name, const uint32_t binding)
    {
        constexpr GLenum glTargets[Texture::TYPE_COUNT] = {GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP};
        glActiveTexture(GL_TEXTURE0 + binding);
        GL_CHECK_ERROR(glBindTexture(glTargets[texture.type()], static_cast<GLuint>(texture.delegate()->id())));

        const GLUniform& uTexture = getUniform(name);
        uTexture.setUniform1i(static_cast<GLint>(binding));
    }

    const GLUniform& getUniform(const String& name)
    {
        if(const auto iter = _uniforms.find(name); iter != _uniforms.end())
            return iter->second;
        _uniforms[name] = getUniformLocation(name);
        return _uniforms[name];
    }

    GLint getUniformLocation(const String& name) const
    {
        const GLint location = glGetUniformLocation(_id, name.c_str());
        CHECK_WARN(location != -1, "Undefined uniform \"%s\". It might be optimized out, or something goes wrong.", name.c_str());
        return location;
    }

    const GLAttribute& getAttribute(const String& name)
    {
        if(const auto iter = _attributes.find(name); iter != _attributes.end())
            return iter->second;
        _attributes[name] = getAttribLocation(name);
        return _attributes[name];
    }

    GLint getAttribLocation(const String& name) const
    {
        return glGetAttribLocation(_id, name.c_str());
    }

    void bindUBOSnapshots(const Vector<RenderBufferSnapshot::UBOSnapshot>& uboSnapshots, const PipelineLayout& shaderLayout)
    {
        size_t binding = 0;
        for(const sp<PipelineLayout::UBO>& ubo : shaderLayout.ubos())
            if(shouldBeBinded(ubo->_stages))
            {
                DCHECK(binding < uboSnapshots.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), shaderLayout.ubos().size());
                const RenderBufferSnapshot::UBOSnapshot& uboSnapshot = uboSnapshots.at(binding++);
                bindUBO(uboSnapshot, ubo);
            }
        _rebind_needed = false;
    }

    bool shouldBeBinded(const enums::ShaderStageSet& stages) const
    {
        return _is_compute_pipeline ? stages.contains(enums::SHADER_STAGE_BIT_COMPUTE) : !stages.contains(enums::SHADER_STAGE_BIT_COMPUTE);
    }

    bool _is_compute_pipeline;
    GLuint _id;

    Map<String, GLAttribute> _attributes;
    Map<String, GLUniform> _uniforms;

    bool _rebind_needed;
};

namespace {

class GLScissor {
public:
    GLScissor(const Optional<Rect>& scissor)
        : _enabled(static_cast<bool>(scissor)) {
        if(_enabled) {
            glEnable(GL_SCISSOR_TEST);
            GL_CHECK_ERROR(glScissor(static_cast<GLint>(scissor->left()), static_cast<GLint>(scissor->top()), static_cast<GLsizei>(scissor->width()), static_cast<GLsizei>(scissor->height())));
        }
    }
    ~GLScissor() {
        if(_enabled)
            glDisable(GL_SCISSOR_TEST);
    }

private:
    bool _enabled;
};

class GLCullFaceTest final : public DrawDecorator {
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

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
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

class GLDepthTest final : public DrawDecorator {
public:
    GLDepthTest(const PipelineDescriptor::TraitDepthTest& trait)
        : _func(GLUtil::toCompareFunc(trait._func)), _pre_func(GL_ZERO), _enabled(trait._enabled), _read_only(!trait._write_enabled), _depth_test_enabled_pre_test(false)
    {
    }

    void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override
    {
        if(_enabled)
        {
            if(_func != GL_ZERO)
            {
                if(_pre_func == GL_ZERO)
                    glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&_pre_func));
                glDepthFunc(_func);
            }
            if(_read_only)
                glDepthMask(GL_FALSE);
        }
        else
        {
            glGetBooleanv(GL_DEPTH_TEST, &_depth_test_enabled_pre_test);
            glDisable(GL_DEPTH_TEST);
        }
    }

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        if(_enabled)
        {
            if(_func != GL_ZERO)
                glDepthFunc(_pre_func);
            if(_read_only)
                glDepthMask(GL_TRUE);
        }
        else if(_depth_test_enabled_pre_test)
            glEnable(GL_DEPTH_TEST);
    }

private:
    GLenum _func;
    GLenum _pre_func;
    bool _enabled;
    bool _read_only;
    GLboolean _depth_test_enabled_pre_test;
};

class GLStencilTest final : public DrawDecorator {
public:
    GLStencilTest(Vector<sp<DrawDecorator>> delegate)
        : _delegate(std::move(delegate)) {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        glEnable(GL_STENCIL_TEST);
        for(const sp<DrawDecorator>& i : _delegate)
            i->preDraw(graphicsContext, context);
    }

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        glStencilMask(0);
        glDisable(GL_STENCIL_TEST);
    }

private:
    Vector<sp<DrawDecorator>> _delegate;
};

class GLStencilTestSeparate final : public DrawDecorator {
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

class GLTraitBlend final : public DrawDecorator {
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
        glBlendFuncSeparate(_src_rgb_factor ? _src_rgb_factor.value() : _src_rgb_factor_default, _dest_rgb_factor ? _dest_rgb_factor.value() : _dest_rgb_factor_default,
                            _src_alpha_factor ? _src_alpha_factor.value() : _src_alpha_factor_default, _dest_alpha_factor ? _dest_alpha_factor.value() : _dest_alpha_factor_default);
    }

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        glBlendFuncSeparate(_src_rgb_factor_default, _dest_rgb_factor_default, _src_alpha_factor_default, _dest_alpha_factor_default);
    }

private:
    Optional<GLenum> _src_rgb_factor;
    Optional<GLenum> _dest_rgb_factor;
    Optional<GLenum> _src_alpha_factor;
    Optional<GLenum> _dest_alpha_factor;

    GLenum _src_rgb_factor_default;
    GLenum _dest_rgb_factor_default;
    GLenum _src_alpha_factor_default;
    GLenum _dest_alpha_factor_default;
};

class GLDrawArrays final : public PipelineDrawCommand {
public:
    GLDrawArrays(const GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        if(drawingContext._draw_count > 0)
        {
            const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
            GL_CHECK_ERROR(glDrawArrays(_mode, param._start * sizeof(element_index_t), static_cast<GLsizei>(drawingContext._draw_count)));
        }
    }

private:
    GLenum _mode;
};

class GLDrawElements final : public PipelineDrawCommand {
public:
    GLDrawElements(const GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        if(drawingContext._draw_count > 0)
        {
            const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
            GL_CHECK_ERROR(glDrawElements(_mode, static_cast<GLsizei>(drawingContext._draw_count), GLIndexType, reinterpret_cast<GLvoid*>(param._start * sizeof(element_index_t))));
        }
    }

private:
    GLenum _mode;
};

class GLDrawElementsInstanced final : public PipelineDrawCommand {
public:
    GLDrawElementsInstanced(const GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const DrawingParams::DrawElementsInstanced& param = drawingContext._parameters.drawElementsInstanced();
        DASSERT(param._count);
        if(drawingContext._draw_count > 0)
        {
            for(const auto& [i, j] : param._instance_buffer_snapshots)
            {
                j.upload(graphicsContext);
                DCHECK(j.id(), "Invaild Instance Buffer: %d", i);
            }
            if(param._start > 0)
                GL_CHECK_ERROR(glDrawElementsInstancedBaseInstance(_mode, static_cast<GLsizei>(param._count), GLIndexType, nullptr, drawingContext._draw_count, param._start));
            else
                GL_CHECK_ERROR(glDrawElementsInstanced(_mode, static_cast<GLsizei>(param._count), GLIndexType, nullptr, drawingContext._draw_count));
        }
    }

private:
    GLenum _mode;
};

class GLMultiDrawElementsIndirect final : public PipelineDrawCommand {
public:
    GLMultiDrawElementsIndirect(const GLenum mode)
        : _mode(mode) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();

        for(const auto& [k, v] : param._instance_buffer_snapshots)
        {
            v.upload(graphicsContext);
            DCHECK(v.id(), "Invaild Divided Buffer Buffer: %d", k);
        }
        param._indirect_cmds.upload(graphicsContext);

        const GLBufferBinder binder(GL_DRAW_INDIRECT_BUFFER, static_cast<GLuint>(param._indirect_cmds.id()));
#ifndef ANDROID
        GL_CHECK_ERROR(glMultiDrawElementsIndirect(_mode, GLIndexType, nullptr, static_cast<GLsizei>(param._indirect_cmd_count), sizeof(DrawingParams::DrawElementsIndirectCommand)));
#else
        for(uint32_t i = 0; i < param._indirect_cmd_count; ++i)
            GL_CHECK_ERROR(glDrawElementsIndirect(_mode, GLIndexType, reinterpret_cast<const void *>(i * sizeof(DrawingParams::DrawElementsIndirectCommand))));
#endif
    }

private:
    GLenum _mode;
};

class GLBufferBaseBinder {
public:
    GLBufferBaseBinder(const GLenum target, const GLuint base, const GLuint buffer)
        : _target(target), _base(base), _buffer(buffer) {
        GL_CHECK_ERROR(glBindBufferBase(_target, _base, _buffer));
    }
    GLBufferBaseBinder(GLBufferBaseBinder&& other)
        : _target(other._target), _base(other._base), _buffer(other._buffer) {
        other._buffer = 0;
    }
    ~GLBufferBaseBinder()
    {
        if(_buffer)
            GL_CHECK_ERROR(glBindBufferBase(_target, _base, 0));
    }
    DISALLOW_COPY_AND_ASSIGN(GLBufferBaseBinder);

private:
    GLenum _target;
    GLuint _base;
    GLuint _buffer;
};

void ensureVertexArray(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    const sp<GLVertexArray>& vertexArray = context._attachments->get<GLVertexArray>();
    uint64_t vertexArrayId = vertexArray ? vertexArray->id() : 0;
    if(!vertexArrayId)
    {
        const sp<Pipeline>& renderPipeline = context._bindings->ensureRenderPipeline(graphicsContext);
        sp<GLVertexArray> va = sp<GLVertexArray>::make(*context._bindings, renderPipeline.cast<GLPipeline>(), context._vertices.delegate());
        va->upload(graphicsContext);
        graphicsContext.renderController()->upload(va, enums::UPLOAD_STRATEGY_ON_SURFACE_READY);
        vertexArrayId = va->id();
        context._attachments->put(std::move(va));
    }
    GL_CHECK_ERROR(glBindVertexArray(static_cast<GLuint>(vertexArrayId)));
}

sp<PipelineDrawCommand> makeBakedRenderer(const PipelineBindings& bindings)
{
    const GLenum mode = GLUtil::toEnum(bindings.drawMode());
    switch(bindings.drawProcedure())
    {
        case enums::DRAW_PROCEDURE_DRAW_ARRAYS:
            return sp<PipelineDrawCommand>::make<GLDrawArrays>(mode);
        case enums::DRAW_PROCEDURE_DRAW_ELEMENTS:
            return sp<PipelineDrawCommand>::make<GLDrawElements>(mode);
        case enums::DRAW_PROCEDURE_DRAW_INSTANCED:
            return sp<PipelineDrawCommand>::make<GLDrawElementsInstanced>(mode);
        case enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT:
            return sp<PipelineDrawCommand>::make<GLMultiDrawElementsIndirect>(mode);
    }
    DFATAL("Not render procedure creator for %d", bindings.drawProcedure());
    return nullptr;
}

class PipelineOperationDraw final : public PipelineOperation {
public:
    PipelineOperationDraw(sp<GLPipeline::Stub> stub, const PipelineBindings& bindings)
        : _stub(std::move(stub)), _scissor(bindings.pipelineDescriptor()->scissor()), _renderer(makeBakedRenderer(bindings)) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        _stub->bind(graphicsContext, drawingContext);

        const GLScissor scissor(drawingContext._scissor ? drawingContext._scissor : _scissor ? Optional<Rect>(_scissor->val()) : Optional<Rect>());

        for(const auto& [i, j] : drawingContext._buffer_snapshot->_ssbos)
            _ssbo_binders.emplace_back(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(i), static_cast<GLuint>(j.id()));

        ensureVertexArray(graphicsContext, drawingContext);
        if(drawingContext._indices)
            GL_CHECK_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(drawingContext._indices.id())));

        _renderer->draw(graphicsContext, drawingContext);
        glBindVertexArray(0);

        _ssbo_binders.clear();
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        DFATAL("This is a drawing pipeline, not compute");
    }

private:
    sp<GLPipeline::Stub> _stub;

    sp<Vec4> _scissor;
    sp<PipelineDrawCommand> _renderer;

    Vector<GLBufferBaseBinder> _ssbo_binders;
};

class PipelineOperationCompute final : public PipelineOperation {
public:
    PipelineOperationCompute(sp<GLPipeline::Stub> stub)
        : _stub(std::move(stub)) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& computeContext) override
    {
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        _stub->bind(graphicsContext, computeContext);

        for(const auto& [i, j] : computeContext._buffer_snapshot->_ssbos)
            _ssbo_binders.emplace_back(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(i), static_cast<GLuint>(j.id()));

        GL_CHECK_ERROR(glDispatchCompute(computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2)));

        _ssbo_binders.clear();

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

private:
    sp<GLPipeline::Stub> _stub;
    Vector<GLBufferBaseBinder> _ssbo_binders;
};

class Stage {
public:
    Stage(sp<Recycler> recycler, const uint32_t version, const GLenum type, const String& source)
        : _recycler(std::move(recycler)), _id(compileShader(version, type, source))
    {
    }
    ~Stage()
    {
        uint32_t id = _id;
        _recycler->recycle([id](GraphicsContext&) {
            LOGD("glDeleteShader(%d)", id);
            glDeleteShader(id);
        });
    }
    DISALLOW_COPY_AND_ASSIGN(Stage);

    uint32_t id() const
    {
        return _id;
    }

private:
    sp<Recycler> _recycler;
    uint32_t _id;
};

bool isComputePipeline(const Map<enums::ShaderStageBit, String>& stages)
{
    if(const auto iter = stages.find(enums::SHADER_STAGE_BIT_COMPUTE); iter != stages.end())
    {
        DCHECK(stages.size() == 1, "Compute shader is an exclusive stage");
        return true;
    }
    return false;
}

String getLinkingInformation(const GLuint id, const Map<enums::ShaderStageBit, String>& stages)
{
    GLint length = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

    const size_t len = static_cast<size_t>(length);
    Vector<GLchar> infos(len + 1);
    glGetProgramInfoLog(id, length, &length, infos.data());
    infos.back() = 0;

    StringBuffer sb;
    for(const auto& v : stages | std::views::values)
        sb << v << std::endl;
    sb << infos.data();
    return sb.str();
}

sp<Stage> makeShader(GraphicsContext& graphicsContext, uint32_t version, GLenum type, const String& source)
{
    typedef Map<GLenum, Map<HashId, WeakPtr<Stage>>> ShaderPool;

    const sp<ShaderPool>& shaderPool = graphicsContext.traits().ensure<ShaderPool>();
    const HashId sourceHash = string_hash(source.c_str());
    if(const auto iter = (*shaderPool)[type].find(sourceHash); iter != (*shaderPool)[type].end())
        if(sp<Stage> shader = iter->second.lock())
            return shader;

    sp<Stage> shader = sp<Stage>::make(graphicsContext.renderController()->recycler(), version, type, source);
    (*shaderPool)[type][sourceHash] = shader;
    return shader;
}

}

GLPipeline::GLPipeline(const sp<Recycler>& recycler, const uint32_t version, Map<enums::ShaderStageBit, String> stages, const PipelineBindings& bindings)
    : _stub(sp<Stub>::make(isComputePipeline(stages))), _recycler(recycler), _version(version), _stages(std::move(stages)), _pipeline_operation(makePipelineOperation(bindings))
{
    for(const auto& [k, v] : bindings.pipelineDescriptor()->configuration()._traits)
    {
        if(k == PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST)
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLCullFaceTest>(std::get<PipelineDescriptor::TraitCullFaceTest>(v)));
        else if(k == PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLDepthTest>(std::get<PipelineDescriptor::TraitDepthTest>(v)));
        else if(k == PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST)
        {
            Vector<sp<DrawDecorator>> delegate;
            const PipelineDescriptor::TraitStencilTest& stencilTest = std::get<PipelineDescriptor::TraitStencilTest>(v);
            if(stencilTest._front._type == PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT && stencilTest._front._type == stencilTest._back._type)
                delegate.push_back(sp<DrawDecorator>::make<GLStencilTestSeparate>(stencilTest._front));
            else
            {
                if(stencilTest._front._type == PipelineDescriptor::FRONT_FACE_TYPE_FRONT)
                    delegate.push_back(sp<DrawDecorator>::make<GLStencilTestSeparate>(stencilTest._front));
                if(stencilTest._back._type == PipelineDescriptor::FRONT_FACE_TYPE_BACK)
                    delegate.push_back(sp<DrawDecorator>::make<GLStencilTestSeparate>(stencilTest._back));
            }
            DASSERT(delegate.size() > 0);
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLStencilTest>(std::move(delegate)));
        }
        else if(k == PipelineDescriptor::TRAIT_TYPE_BLEND)
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLTraitBlend>(std::get<PipelineDescriptor::TraitBlend>(v)));
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
    const GLuint id = glCreateProgram();

    _stub->_rebind_needed = true;
    _stub->_id = id;

    sp<Stage> compiledStages[enums::SHADER_STAGE_BIT_COUNT];
    for(const auto& [k, v] : _stages)
    {
        sp<Stage>& stage = compiledStages[k];
        stage = makeShader(graphicsContext, _version, GLUtil::toShaderType(k), v);
        GL_CHECK_ERROR(glAttachShader(id, stage->id()));
    }

    GL_CHECK_ERROR(glLinkProgram(id));

    for(const auto& i : compiledStages)
        if(i)
            GL_CHECK_ERROR(glDetachShader(id, i->id()));

    GLint linkstatus = 0;
    GL_CHECK_ERROR(glGetProgramiv(id, GL_LINK_STATUS, &linkstatus));
    CHECK(linkstatus, "Program link failed: %s", getLinkingInformation(id, _stages).c_str());
    GLint programSize = 0;
    GL_CHECK_ERROR(glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &programSize));
    Vector<uint8_t> binaryData(programSize);
    GLenum binaryFormat;
    GL_CHECK_ERROR(glGetProgramBinary(id, programSize, &programSize, &binaryFormat, binaryData.data()));
    LOGD("Compile and link success, program size: %d", programSize);
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

void GLPipeline::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    for(const sp<DrawDecorator>& i : _draw_decorators)
        i->preDraw(graphicsContext, drawingContext);
    _pipeline_operation->draw(graphicsContext, drawingContext);
    for(const sp<DrawDecorator>& i : _draw_decorators)
        i->postDraw(graphicsContext, drawingContext);
}

void GLPipeline::compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    _pipeline_operation->compute(graphicsContext, computeContext);
}

void GLPipeline::bindBuffer(GraphicsContext& graphicsContext, const PipelineLayout& shaderLayout, const Vector<std::pair<uint32_t, Buffer>>& streams)
{
    DCHECK(id(), "GLProgram unprepared");
    bindBuffer(graphicsContext, shaderLayout, 0);
    for(const auto& [i, j] : streams)
    {
        if(!j.id())
            j.upload(graphicsContext);

        const GLBufferBinder binder(GL_ARRAY_BUFFER, static_cast<GLuint>(j.id()));
        bindBuffer(graphicsContext, shaderLayout, i);
    }
}

void GLPipeline::activeTexture(const Texture& texture, const String& name, const uint32_t binding) const
{
    _stub->activeTexture(texture, name, binding);
}

const GLPipeline::GLUniform& GLPipeline::getUniform(const String& name) const
{
    return _stub->getUniform(name);
}

void GLPipeline::bindBuffer(GraphicsContext& /*graphicsContext*/, const PipelineLayout& shaderLayout, const uint32_t divisor) const
{
    const PipelineLayout::StreamLayout& stream = shaderLayout.getStreamLayout(divisor);
    for(const Attribute& i : stream.attributes().values())
    {
        const GLAttribute& glAttribute = _stub->getAttribute(i.name());
        glAttribute.bind(i, stream.stride());
    }
}

sp<PipelineOperation> GLPipeline::makePipelineOperation(const PipelineBindings& pipelineBindings) const
{
    return _stub->_is_compute_pipeline ?  sp<PipelineOperation>::make<PipelineOperationCompute>(_stub) : sp<PipelineOperation>::make<PipelineOperationDraw>(_stub, pipelineBindings);
}

GLPipeline::GLUniform::GLUniform(const GLint location)
    : _location(location)
{
}

GLPipeline::GLUniform::operator bool() const
{
    return _location != -1;
}

void GLPipeline::GLUniform::setUniform1i(const GLint x) const
{
    GL_CHECK_ERROR(glUniform1i(_location, x));
}

void GLPipeline::GLUniform::setUniform2i(const GLint x, const GLint y) const
{
    GL_CHECK_ERROR(glUniform2i(_location, x, y));
}

void GLPipeline::GLUniform::setUniform1f(const GLfloat x) const
{
    GL_CHECK_ERROR(glUniform1f(_location, x));
}

void GLPipeline::GLUniform::setUniform2f(const GLfloat x, const GLfloat y) const
{
    GL_CHECK_ERROR(glUniform2f(_location, x, y));
}

void GLPipeline::GLUniform::setUniform3f(const GLfloat x, const GLfloat y, const GLfloat z) const
{
    GL_CHECK_ERROR(glUniform3f(_location, x, y, z));
}

void GLPipeline::GLUniform::setUniform4f(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a) const
{
    GL_CHECK_ERROR(glUniform4f(_location, r, g, b, a));
}

void GLPipeline::GLUniform::setUniform4fv(const GLsizei count, const GLfloat* value) const
{
    GL_CHECK_ERROR(glUniform4fv(_location, count, value));
}

void GLPipeline::GLUniform::setUniformMatrix4fv(const GLsizei count, const GLboolean transpose, const GLfloat* value) const
{
    GL_CHECK_ERROR(glUniformMatrix4fv(_location, count, transpose, value));
}

}
