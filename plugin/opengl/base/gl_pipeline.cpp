#include "opengl/base/gl_pipeline.h"

#include "core/types/weak_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"
#include "opengl/impl/es30/gl_resource/gl_vertex_array.h"

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

#include "opengl/util/gl_debug.h"
#include "opengl/util/gl_util.h"

#include "platform/platform.h"

namespace ark::plugin::opengl {

namespace {

void setVertexPointer(const Attribute& attribute, const GLuint location, const GLsizei stride, const uint32_t length, const uint32_t offset)
{
    constexpr GLenum glTypes[Attribute::TYPE_COUNT] = {GL_BYTE, GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_SHORT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT};
    glEnableVertexAttribArray(location);
    if(attribute.type() == Attribute::TYPE_FLOAT || attribute.normalized())
        glVertexAttribPointer(location, length, glTypes[attribute.type()], attribute.normalized() ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
    else
        glVertexAttribIPointer(location, length, glTypes[attribute.type()], stride, reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));

    if(attribute.divisor())
        glVertexAttribDivisor(location, attribute.divisor());
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
        CHECK_WARN(_location >= 0, "Attribute \"%s\" location: %d", attribute.name().c_str(), _location);
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
        glUseProgram(_id);

        const PipelineBindings& pipelineBindings = pipelineContext._bindings;
        bindUBOSnapshots(pipelineContext._buffer_object->_ubos, pipelineBindings.pipelineLayout());

        uint32_t binding = 0;
        const Vector<String>& samplerNames = pipelineBindings.pipelineLayout()->samplers().keys();
        const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& samplers = pipelineBindings.samplers();
        DASSERT(samplerNames.size() == samplers.size());
        for(size_t i = 0; i < samplerNames.size(); ++i)
        {
            const String& name = samplerNames.at(i);
            const sp<Texture>& texture = samplers.at(i).first;
            CHECK_WARN(texture, "Pipeline has unbound sampler \"%s\"", name.c_str());
            if(texture)
                activeTexture(texture, name, binding);
            ++ binding;
        }

        const Vector<String>& imageNames = pipelineBindings.pipelineLayout()->images().keys();
        const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& images = pipelineBindings.images();
        for(size_t i = 0; i < images.size(); ++i)
            if(const sp<Texture>& image = images.at(i).first)
            {
                const String& name = imageNames.at(i);
                bindImage(image, name, static_cast<uint32_t>(i));
            }
    }

    void bindUBO(const RenderLayerSnapshot::UBOSnapshot& uboSnapshot, const PipelineLayout::UBO& ubo)
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
        glBindImageTexture(binding, static_cast<GLuint>(texture.delegate()->id()), 0, GL_FALSE, 0, GL_READ_WRITE, format);
    }

    void activeTexture(const Texture& texture, const String& name, const uint32_t binding)
    {
        constexpr GLenum glTargets[Texture::TYPE_COUNT] = {GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP};
        glActiveTexture(GL_TEXTURE0 + binding);
        glBindTexture(glTargets[texture.type()], static_cast<GLuint>(texture.delegate()->id()));

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

    void bindUBOSnapshots(const Vector<RenderLayerSnapshot::UBOSnapshot>& uboSnapshots, const PipelineLayout& shaderLayout)
    {
        size_t binding = 0;
        for(const sp<PipelineLayout::UBO>& ubo : shaderLayout.ubos())
            if(shouldBeBinded(ubo->_stages))
            {
                DCHECK(binding < uboSnapshots.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), shaderLayout.ubos().size());
                const RenderLayerSnapshot::UBOSnapshot& uboSnapshot = uboSnapshots.at(binding++);
                bindUBO(uboSnapshot, ubo);
            }
        _rebind_needed = false;
    }

    bool shouldBeBinded(const ShaderStageSet& stages) const
    {
        return _is_compute_pipeline ? stages.has(Enum::SHADER_STAGE_BIT_COMPUTE) : !stages.has(Enum::SHADER_STAGE_BIT_COMPUTE);
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

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
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
        const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
        DASSERT(drawingContext._draw_count);
        glDrawArrays(_mode, param._start * sizeof(element_index_t), static_cast<GLsizei>(drawingContext._draw_count));
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
        const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
        DASSERT(drawingContext._draw_count);
        glDrawElements(_mode, static_cast<GLsizei>(drawingContext._draw_count), GLIndexType, reinterpret_cast<GLvoid*>(param._start * sizeof(element_index_t)));
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

class GLMultiDrawElementsIndirect final : public PipelineDrawCommand {
public:
    GLMultiDrawElementsIndirect(const GLenum mode)
        : _mode(mode) {
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

private:
    GLenum _mode;
};

class GLBufferBaseBinder {
public:
    GLBufferBaseBinder(GLenum target, GLuint base, GLuint buffer)
        : _target(target), _base(base), _buffer(buffer) {
        glBindBufferBase(_target, _base, _buffer);
    }
    GLBufferBaseBinder(GLBufferBaseBinder&& other)
        : _target(other._target), _base(other._base), _buffer(other._buffer) {
        other._buffer = 0;
    }
    ~GLBufferBaseBinder()
    {
        if(_buffer)
            glBindBufferBase(_target, _base, 0);
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
    if(!vertexArrayId) {
        const sp<Pipeline>& renderPipeline = context._bindings->ensureRenderPipeline(graphicsContext);
        sp<GLVertexArray> va = sp<GLVertexArray>::make(*context._bindings, renderPipeline.cast<GLPipeline>(), context._vertices.delegate());
        va->upload(graphicsContext);
        graphicsContext.renderController()->upload(va, RenderController::US_ON_SURFACE_READY);
        vertexArrayId = va->id();
        context._attachments->put(std::move(va));
    }
    glBindVertexArray(static_cast<GLuint>(vertexArrayId));
}

sp<PipelineDrawCommand> makeBakedRenderer(const PipelineBindings& bindings)
{
    const GLenum mode = GLUtil::toEnum(bindings.drawMode());
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

class PipelineOperationDraw final : public PipelineOperation {
public:
    PipelineOperationDraw(sp<GLPipeline::Stub> stub, const PipelineBindings& bindings)
        : _stub(std::move(stub)), _scissor(bindings.pipelineDescriptor()->scissor()), _renderer(makeBakedRenderer(bindings)) {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        _stub->bind(graphicsContext, drawingContext);

        const GLScissor scissor(drawingContext._scissor ? drawingContext._scissor : _scissor ? Optional<Rect>(_scissor->val()) : Optional<Rect>());

        for(const auto& [i, j] : drawingContext._buffer_object->_ssbos)
            _ssbo_binders.emplace_back(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(i), static_cast<GLuint>(j.id()));

        ensureVertexArray(graphicsContext, drawingContext);
        if(drawingContext._indices)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(drawingContext._indices.id()));

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

        for(const auto& [i, j] : computeContext._buffer_object->_ssbos)
            _ssbo_binders.emplace_back(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(i), static_cast<GLuint>(j.id()));

        glDispatchCompute(computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));

        _ssbo_binders.clear();
    }

private:
    sp<GLPipeline::Stub> _stub;
    Vector<GLBufferBaseBinder> _ssbo_binders;
};

class Stage {
public:
    Stage(sp<Recycler> recycler, const uint32_t version, const GLenum type, const String& source)
        : _recycler(std::move(recycler)), _id(compileShader(version, type, source)) {
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

bool isComputePipeline(const Map<Enum::ShaderStageBit, String>& stages)
{
    if(const auto iter = stages.find(Enum::SHADER_STAGE_BIT_COMPUTE); iter != stages.end())
    {
        DCHECK(stages.size() == 1, "Compute shader is an exclusive stage");
        return true;
    }
    return false;
}

String getInformationLog(GLuint id)
{
    GLint length = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

    const size_t len = static_cast<size_t>(length);
    Vector<GLchar> infos(len + 1);
    glGetProgramInfoLog(id, length, &length, infos.data());
    infos.back() = 0;
    return infos.data();
}

sp<Stage> makeShader(GraphicsContext& graphicsContext, uint32_t version, GLenum type, const String& source)
{
    typedef HashMap<GLenum, Map<String, WeakPtr<Stage>>> ShaderPool;

    const sp<ShaderPool>& shaders = graphicsContext.traits().ensure<ShaderPool>();
    if(const auto iter = (*shaders)[type].find(source); iter != (*shaders)[type].end())
        if(const sp<Stage> shader = iter->second.lock())
            return shader;

    const sp<Stage> shader = sp<Stage>::make(graphicsContext.renderController()->recycler(), version, type, source);
    (*shaders)[type][source] = shader;
    return shader;
}

}

GLPipeline::GLPipeline(const sp<Recycler>& recycler, const uint32_t version, Map<Enum::ShaderStageBit, String> stages, const PipelineBindings& bindings)
    : _stub(sp<Stub>::make(isComputePipeline(stages))), _recycler(recycler), _version(version), _stages(std::move(stages)), _pipeline_operation(makePipelineOperation(bindings))
{
    for(const auto& [k, v] : bindings.pipelineDescriptor()->configuration()._traits)
    {
        if(k == PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST)
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLCullFaceTest>(v._cull_face_test));
        else if(k == PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLDepthTest>(v._depth_test));
        else if(k == PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST)
        {
            Vector<sp<DrawDecorator>> delegate;
            const PipelineDescriptor::TraitStencilTest& test = v._stencil_test;
            if(test._front._type == PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT && test._front._type == test._back._type)
                delegate.push_back(sp<DrawDecorator>::make<GLStencilTestSeparate>(test._front));
            else
            {
                if(test._front._type == PipelineDescriptor::FRONT_FACE_TYPE_FRONT)
                    delegate.push_back(sp<DrawDecorator>::make<GLStencilTestSeparate>(test._front));
                if(test._back._type == PipelineDescriptor::FRONT_FACE_TYPE_BACK)
                    delegate.push_back(sp<DrawDecorator>::make<GLStencilTestSeparate>(test._back));
            }
            DASSERT(delegate.size() > 0);
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLStencilTest>(std::move(delegate)));
        }
        else if(k == PipelineDescriptor::TRAIT_TYPE_BLEND)
            _draw_decorators.push_back(sp<DrawDecorator>::make<GLTraitBlend>(v._blend));
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

    Map<Enum::ShaderStageBit, sp<Stage>> compiledStages;
    for(const auto& [k, v] : _stages)
    {
        sp<Stage>& shader = compiledStages[k];
        shader = makeShader(graphicsContext, _version, GLUtil::toShaderType(k), v);
        glAttachShader(id, shader->id());
    }

    glLinkProgram(id);

    for(const auto& i : compiledStages)
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

void GLPipeline::bindBuffer(GraphicsContext& graphicsContext, const PipelineLayout& shaderLayout, const Map<uint32_t, Buffer>& streams)
{
    DCHECK(id(), "GLProgram unprepared");
    bindBuffer(graphicsContext, shaderLayout, 0);
    for(const auto& [i, j] : streams)
    {
        if(!j.id())
            j.upload(graphicsContext);

        const volatile GLBufferBinder binder(GL_ARRAY_BUFFER, static_cast<GLuint>(j.id()));
        bindBuffer(graphicsContext, shaderLayout, i);
    }
}

void GLPipeline::activeTexture(const Texture& texture, const String& name, uint32_t binding)
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
    glUniform1i(_location, x);
}

void GLPipeline::GLUniform::setUniform1f(const GLfloat x) const
{
    glUniform1f(_location, x);
}

void GLPipeline::GLUniform::setUniform2f(const GLfloat x, const GLfloat y) const
{
    glUniform2f(_location, x, y);
}

void GLPipeline::GLUniform::setUniform3f(const GLfloat x, const GLfloat y, const GLfloat z) const
{
    glUniform3f(_location, x, y, z);
}

void GLPipeline::GLUniform::setUniform4f(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a) const
{
    glUniform4f(_location, r, g, b, a);
}

void GLPipeline::GLUniform::setUniform4fv(const GLsizei count, const GLfloat* value) const
{
    glUniform4fv(_location, count, value);
}

void GLPipeline::GLUniform::setUniformMatrix4fv(const GLsizei count, const GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(_location, count, transpose, value);
}

}
